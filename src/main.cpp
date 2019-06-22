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
  sub_run->add_option("solver", solver_name, "the solver name");
  sub_run->add_option("--desc", desc_filename, "*.desc file input");
  sub_run->add_option("--map", map_filename, "*.map file input");
  sub_run->add_option("--output", command_output_filename, "output commands to a file");

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
    Game game(str);

    for (auto line : dumpMapString(game.map2d, game.wrappy)) {
      std::cout << line << std::endl;
    }
  }

  // ================== run
  if (sub_run->parsed()) {
    Game game; 
    if (std::experimental::filesystem::is_regular_file(desc_filename)) {
      std::ifstream ifs(desc_filename);
      std::string str((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
      game = Game(str);
    } else if (std::experimental::filesystem::is_regular_file(map_filename)) {
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(ifs, l);)
        input.emplace_back(l);
      game = Game(input);
    } else {
      // read *.map from stdin
      std::ifstream ifs(map_filename);
      std::vector<std::string> input;
      for (std::string l; std::getline(std::cin, l);)
        input.emplace_back(l);
      game = Game(input);
    }

    // Do something
    SolverFunction solver = SolverRegistry::getSolver(solver_name);
    if (solver) {
      std::string command = solver(game);
      std::cout << "command: " << command << "\n";
    }

    // Test output
    std::cout << game << "\n";

    // command output
    if (!command_output_filename.empty()) {
      std::ofstream ofs(command_output_filename);
      ofs << game.getCommand();
    }
  }

  return 0;
}
