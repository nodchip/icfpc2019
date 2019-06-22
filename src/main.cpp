#include <cassert>
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <experimental/filesystem>

#include <CLI/CLI.hpp>

#include "game.h"
#include "puzzle.h"
#include "fill_polygon.h"
#include "solver_registry.h"

int main(int argc, char* argv[]) {
  CLI::App app { "main module" };
  app.require_subcommand();

  std::string solver_name = "simple";
  std::string desc_filename;
  std::string map_filename;
  std::string command_output_filename;
  std::string meta_output_filename;

  auto sub_list_solvers = app.add_subcommand("list_solvers", "list up registered solvers");

  auto sub_convert = app.add_subcommand("convert", "read *.desc file and print map format");
  sub_convert->add_option("input_desc", desc_filename, "*.desc file input");

  auto sub_run = app.add_subcommand("run");
  SolverParam solver_param;
  sub_run->add_option("solver", solver_name, "the solver name");
  sub_run->add_option("--desc", desc_filename, "*.desc file input");
  sub_run->add_option("--map", map_filename, "*.map file input");
  sub_run->add_option("--output", command_output_filename, "output commands to a file");
  sub_run->add_option("--meta", meta_output_filename, "output meta information to a JSON file");
  sub_run->add_option("--wait-ms", solver_param.wait_ms, "display and pause a while between frames");

  std::string cond_filename;

  auto sub_puzzle_convert = app.add_subcommand("puzzle_convert", "read *.cond file and print pmap format");
  sub_puzzle_convert->add_option("input_cond", cond_filename, "*.cond file input");

  auto sub_puzzle_run = app.add_subcommand("puzzle_run", "solve a puzzle");
  PuzzleSolverParam puzzle_solver_param;
  sub_puzzle_run->add_option("solver", solver_name, "the solver name");
  sub_puzzle_run->add_option("--cond", cond_filename, "*.cond file input");
  sub_puzzle_run->add_option("--output", command_output_filename, "output commands to a file");
  sub_puzzle_run->add_option("--meta", meta_output_filename, "output meta information to a JSON file");

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
    std::unique_ptr<Game> game; 
    if (std::experimental::filesystem::is_regular_file(desc_filename)) {
      std::ifstream ifs(desc_filename);
      std::string str((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
      game.reset(new Game(str));
    } else if (std::experimental::filesystem::is_regular_file(map_filename)) {
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(ifs, l);)
        input.emplace_back(l);
      game.reset(new Game(input));
    } else {
      // read *.map from stdin
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(std::cin, l);)
        input.emplace_back(l);
      game.reset(new Game(input));
    }

    // Do something
    const auto t0 = std::chrono::system_clock::now();
    if (SolverFunction solver = SolverRegistry<SolverFunction>::getSolver(solver_name)) {
      solver(solver_param, game.get());
      if (!game->isEnd()) {
        std::cerr << "******** Some cells are not wrapped **********\n"
                  << *game << "\n";
      }
    }
    const auto t1 = std::chrono::system_clock::now();
    const double solve_s = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;

    // command output
    if (!command_output_filename.empty()) {
      std::ofstream ofs(command_output_filename);
      ofs << game->getCommand();
    }
    // meta information output
    if (!meta_output_filename.empty() && game->isEnd()) {
      std::ofstream ofs(meta_output_filename);
      ofs << "{\"name\":\"" << solver_name
          << "\",\"time_unit\":" << game->time
          << "\",\"wall_clock_time\":" << solve_s << "}\n";
    }
    std::cout << "Time step: " << game->time << "\n";
    std::cout << "Elapsed  : " << solve_s << " s\n";
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
    Polygon simple_polygon;
    const auto t0 = std::chrono::system_clock::now();
    if (PuzzleSolverFunction solver = SolverRegistry<PuzzleSolverFunction>::getSolver(solver_name)) {
      simple_polygon = solver(puzzle_solver_param, puzzle);

      // TODO: validation.
    }
    const auto t1 = std::chrono::system_clock::now();
    const double solve_s = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() * 1e-6;

    // output
    if (!command_output_filename.empty()) {
      std::ofstream ofs(command_output_filename);
      for (auto p : simple_polygon) {
        ofs << p;
      }
    } else {
      for (auto p : simple_polygon) {
        std::cout << p;
      }
      std::cout << std::endl;
    }

    // meta information output
    if (!meta_output_filename.empty()) {
      std::ofstream ofs(meta_output_filename);
      ofs << "{\"name\":\"" << solver_name
          << "\",\"wall_clock_time\":" << solve_s << "}\n";
    }
  }

  return 0;
}
