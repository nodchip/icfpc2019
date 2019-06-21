#include <iostream>
#include <vector>
#include <string>

#include "game.h"

int main(int argc, char* argv[]) {

  // test input: part-1-example/example-01.map
  // TODO(peria): Make Map parse .desc well.
  std::vector<std::string> input;
  for (std::string l; std::getline(std::cin, l);)
    input.emplace_back(l);
  Game game(input);

  // Do something

  // Test output
  std::cout << game << "\n";

  return 0;
}
