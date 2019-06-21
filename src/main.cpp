#include <iostream>
#include <vector>
#include <string>

#include "map.h"
#include "wrappy.h"

int main(int argc, char* argv[]) {

  // test input;
  /*
    "X........."
    "......#..."
    ".........."
    ".....##..."
    ".....##..."
    ".....##..."
    "L....##..."
    "FF...##..."
    "BB........"
    "@........."
   */
  std::vector<std::string> input;
  for (std::string l; std::getline(std::cin, l);)
    input.emplace_back(l);
  Map mp(input);
  Wrappy wrappy(mp);

  return 0;
}
