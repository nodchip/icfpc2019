#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <regex>
#include <experimental/filesystem>

#include <CLI/CLI.hpp>

#include "game.h"
#include "puzzle.h"
#include "fill_polygon.h"
#include "solver_registry.h"

int parseProblemNumber(std::string desc_or_map_file_path) {
  std::regex re(R"(prob-(\d{3}))");
  std::smatch m;
  if (std::regex_search(desc_or_map_file_path, m, re)) {
    assert (m.size() == 2);
    return std::stoi(m[1].str());
  }
  return -1;
}

std::string resolveDescPath(std::string desc_path_hint) {
  // parse various input:
  // ../dataset/problems/prob-001.desc
  // prob-001
  // 001
  std::vector<std::string> candidates = {
    desc_path_hint,
    std::string("../dataset/problems/") + desc_path_hint,
    std::string("../dataset/problems/") + desc_path_hint + std::string(".desc"),
    std::string("../dataset/problems/prob-") + desc_path_hint + std::string(".desc"),
  };
  for (auto c : candidates) {
    if (std::experimental::filesystem::is_regular_file(c)) {
      return c;
    }
  }
  return desc_path_hint;
}

const std::string& toString(const std::string& s) {
  return s;
}

std::string toString(const std::wstring& s) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> cv;
  return cv.to_bytes(s);
}

int main(int argc, char* argv[]) {
  int return_code = 0;
  CLI::App app { "main module" };
  app.require_subcommand();

  std::string solver_name;
  std::string desc_filename;
  std::string map_filename;
  std::string command_output_filename;
  std::string meta_output_filename;

  auto sub_list_solvers = app.add_subcommand("list_solvers", "list up registered solvers");

  auto sub_convert = app.add_subcommand("convert", "read *.desc file and print map format");
  sub_convert->add_option("input_desc", desc_filename, "*.desc file input");

  auto sub_run = app.add_subcommand("run");
  SolverParam solver_param;
  std::string buy_database_dir;
  std::string buy_str;
  sub_run->add_option("solver", solver_name, "the solver name");
  sub_run->add_option("--desc", desc_filename, "*.desc file input");
  sub_run->add_option("--map", map_filename, "*.map file input");
  sub_run->add_option("--output", command_output_filename, "output commands to a file");
  sub_run->add_option("--meta", meta_output_filename, "output meta information to a JSON file");
  sub_run->add_option("--buy", buy_database_dir, "use a buy directory");
  sub_run->add_option("--buy-str", buy_str, "buy string. e.g.) BBBRRLFC");
  sub_run->add_option("--wait-ms", solver_param.wait_ms, "display and pause a while between frames");

  auto sub_check_command = app.add_subcommand("check_command");
  std::string solution_filename;
  sub_check_command->add_option("solution_file", solution_filename, "input .sol file");

  std::string cond_filename;

  auto sub_puzzle_convert = app.add_subcommand("puzzle_convert", "read *.cond file and print pmap format");
  sub_puzzle_convert->add_option("input_cond", cond_filename, "*.cond file input");

  auto sub_puzzle_run = app.add_subcommand("puzzle_run", "solve a puzzle");
  PuzzleSolverParam puzzle_solver_param;
  bool puzzle_validation = false;
  sub_puzzle_run->add_option("solver", solver_name, "the solver name");
  sub_puzzle_run->add_option("--cond", cond_filename, "*.cond file input");
  sub_puzzle_run->add_option("--output", command_output_filename, "output commands to a file");
  sub_puzzle_run->add_option("--meta", meta_output_filename, "output meta information to a JSON file");
  sub_puzzle_run->add_flag("--validate", puzzle_validation, "validate puzzle solution");

  CLI11_PARSE(app, argc, argv);

  // ================== list_solvers
  if (sub_list_solvers->parsed()) {
    std::cout << "== game solvers ==" << std::endl;
    SolverRegistry<SolverFunction>::displaySolvers();
    std::cout << "== puzzle solvers ==" << std::endl;
    SolverRegistry<PuzzleSolverFunction>::displaySolvers();
    return 0;
  }

  // ================== convert
  if (sub_convert->parsed()) {
    desc_filename = resolveDescPath(desc_filename);
    assert (std::experimental::filesystem::is_regular_file(desc_filename));
    std::ifstream ifs(desc_filename);
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    Game game(str);

    for (auto line : dumpMapString(game.map2d, game.getWrapperPositions())) {
      std::cout << line << std::endl;
    }

    return 0;
  }

  // ================== run
  if (sub_run->parsed()) {
    std::string stem;
    std::unique_ptr<Game> game; 
    desc_filename = resolveDescPath(desc_filename);
    int problem_no = -1;
    if (std::experimental::filesystem::is_regular_file(desc_filename)) {
      std::cerr << "Input: " << desc_filename << "\n";
      problem_no = parseProblemNumber(desc_filename);
      stem = toString(std::experimental::filesystem::path(desc_filename).stem());
      std::ifstream ifs(desc_filename);
      std::string str((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
      game.reset(new Game(str));
    } else if (std::experimental::filesystem::is_regular_file(map_filename)) {
      std::cerr << "Input: " << map_filename << "\n";
      problem_no = parseProblemNumber(map_filename);
      stem = toString(std::experimental::filesystem::path(map_filename).stem());
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(ifs, l);)
        input.emplace_back(l);
      game.reset(new Game(input));
    } else {
      // read *.map from stdin
      std::cerr << "Input: stdin" << "\n";
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(std::cin, l);)
        input.emplace_back(l);
      game.reset(new Game(input));
    }
    game->problem_no = problem_no;

    assert (buy_database_dir.empty() || buy_str.empty()); // mutially exclusive options.
    Buy buy;
    if (!stem.empty() && std::experimental::filesystem::is_directory(buy_database_dir)) {
      // read buy file.
      std::string buy_path = buy_database_dir + "/" + stem + ".buy";
      if (!std::experimental::filesystem::is_regular_file(buy_path)) {
        std::cerr << "**** no buy file [" << buy_path << "] for " << stem << std::endl;
      } else {
        std::cerr << "**** use buy file [" << buy_path << "] for " << stem << std::endl;
        buy = Buy::fromFile(buy_path);
      }
    }
    if (!buy_str.empty()) {
      std::cerr << "**** use buy str [" << buy_str << "]" << std::endl;
      buy = Buy(buy_str);
    }
    if (!buy.empty()) {
      game->buyBoosters(buy);
    }

    // solve the task.
    const auto t0 = std::chrono::system_clock::now();
    if (SolverFunction solver = SolverRegistry<SolverFunction>::getSolver(solver_name)) {
      solver(solver_param, game.get(), [](Game*) { return true; });
      if (!game->isEnd()) {
        std::cerr << "******** Some cells are not wrapped **********\n"
                  << *game << "\n";
      }
    }
    const auto t1 = std::chrono::system_clock::now();
    const double solve_s = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;

    // check suspicous commands.
    checkCommandString(game->getCommand());

    // command output
    if (!command_output_filename.empty()) {
      std::ofstream ofs(command_output_filename);
      if (!buy.empty()) { // to distinguish from non-buy solutions.
        ofs << "buy:" << buy.toString() << "\n";
      }
      ofs << game->getCommand();
    }
    // meta information output
    if (!meta_output_filename.empty() && game->isEnd()) {
      std::ofstream ofs(meta_output_filename);
      ofs << "{\"name\":\"" << solver_name
          << "\",\"time_unit\":" << game->time
          << ",\"buy\":\"" << buy.toString()
          << "\",\"wall_clock_time\":" << solve_s << "}\n";
    }
    std::cout << "Time step: " << game->time << "\n";
    std::cout << "Elapsed  : " << solve_s << " s\n";
  }

  if (sub_check_command->parsed()) {
    assert (std::experimental::filesystem::is_regular_file(solution_filename));
    std::ifstream ifs(solution_filename);
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    std::cerr << "Command: " << str << std::endl;
    if (checkCommandString(str)) {
      std::cerr << "[O] the command looks OK!" << std::endl;
      return_code = 0;
    } else {
      std::cerr << "[X] the command is suspicious!" << std::endl;
      return_code = 1;
    }
  }

  // ================== puzzle_convert
  if (sub_puzzle_convert->parsed()) {
    assert (std::experimental::filesystem::is_regular_file(cond_filename));
    std::ifstream ifs(cond_filename);
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    Puzzle puzzle = parsePuzzleCondString(str);
    for (auto line : dumpPuzzleConstraintMapString(puzzle.constraintsToMap())) {
      std::cout << line << std::endl;
    }
  }

  // ================== puzzle_run
  if (sub_puzzle_run->parsed()) {
    assert (std::experimental::filesystem::is_regular_file(cond_filename));
    std::ifstream ifs(cond_filename);
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    Puzzle puzzle = parsePuzzleCondString(str);

    // solve
    PuzzleSolution puzzle_solution;
    const auto t0 = std::chrono::system_clock::now();
    if (PuzzleSolverFunction solver = SolverRegistry<PuzzleSolverFunction>::getSolver(solver_name)) {
      puzzle_solution = solver(puzzle_solver_param, puzzle);

      if (puzzle_validation) {
        if (puzzle.validateSolution(puzzle_solution)) {
          std::cerr << "validation succeeded!" << std::endl;
          return_code = 0;
        } else {
          std::cerr << "validation failed!" << std::endl;
          return_code = 1;
        }
      } else {
        std::cerr << "no validation." << std::endl;
      }
    }
    const auto t1 = std::chrono::system_clock::now();
    const double solve_s = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;

    // output
    if (!command_output_filename.empty()) {
      std::ofstream ofs(command_output_filename);
      ofs << puzzle_solution.toString();
    } else {
      std::cout << puzzle_solution.toString() << std::endl;
    }

    // meta information output
    if (!meta_output_filename.empty()) {
      std::ofstream ofs(meta_output_filename);
      ofs << "{\"name\":\"" << solver_name
          << "\",\"wall_clock_time\":" << solve_s << "}\n";
    }
  }

  return return_code;
}
