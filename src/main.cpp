#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cassert>
#include <experimental/filesystem>

#include <CLI/CLI.hpp>

#include "game.h"
#include "solver_registry.h"

int main(int argc, char* argv[]) {
  CLI::App app { "main module" };
  app.require_subcommand();

  std::string solver_name = "simple";
  std::string desc_filename;
  std::string map_filename;
  std::string command_output_filename;

  auto sub_list_solvers = app.add_subcommand("list_solvers", "list up registered solvers");

  auto sub_convert = app.add_subcommand("convert", "read *.desc file and print map format");
  sub_convert->add_option("input_desc", desc_filename, "*.desc file input");

  auto sub_run = app.add_subcommand("run");
  SolverParam solver_param;
  sub_run->add_option("solver", solver_name, "the solver name");
  sub_run->add_option("--desc", desc_filename, "*.desc file input");
  sub_run->add_option("--map", map_filename, "*.map file input");
  sub_run->add_option("--output", command_output_filename, "output commands to a file");
  sub_run->add_option("--wait-ms", solver_param.wait_ms, "display and pause a while between frames");

  CLI11_PARSE(app, argc, argv);

  // ================== list_solvers
  if (sub_list_solvers->parsed()) {
    SolverRegistry::displaySolvers();
  }

  // ================== convert
  if (sub_convert->parsed()) {
    assert (std::experimental::filesystem::is_regular_file(desc_filename));
    std::ifstream ifs(desc_filename);
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    Game::Ptr game = std::make_shared<Game>(str);

    for (auto line : dumpMapString(game->map2d, game->getWrapperPositions())) {
      std::cout << line << std::endl;
    }
  }

  // ================== run
  if (sub_run->parsed()) {
    Game::Ptr game; 
    if (std::experimental::filesystem::is_regular_file(desc_filename)) {
      std::ifstream ifs(desc_filename);
      std::string str((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
      game = std::make_shared<Game>(str);
    } else if (std::experimental::filesystem::is_regular_file(map_filename)) {
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(ifs, l);)
        input.emplace_back(l);
      game = std::make_shared<Game>(input);
    } else {
      // read *.map from stdin
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(std::cin, l);)
        input.emplace_back(l);
      game = std::make_shared<Game>(input);
    }

    // Do something
    if (SolverFunction solver = SolverRegistry::getSolver(solver_name)) {
      solver(solver_param, game);
    }
    // TODO: Check if no unwrapped cells are remained.

    // command output
    if (!command_output_filename.empty()) {
      std::ofstream ofs(command_output_filename);
      ofs << game->getCommand();
    }
    std::cout << "Time: " << game->time << "\n";
  }

  return 0;
}
