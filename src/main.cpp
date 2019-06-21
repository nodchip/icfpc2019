#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <experimental/filesystem>

#include <CLI/CLI.hpp>

#include "game.h"

int main(int argc, char* argv[]) {
  CLI::App app { "main module" };

  std::string desc_filename;
  std::string map_filename;
  app.add_option("--desc", desc_filename, "*.desc file input");
  app.add_option("--map", map_filename, "*.map file input");

  CLI11_PARSE(app, argc, argv);

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

  // Test output
  std::cout << game << "\n";

  return 0;
}
