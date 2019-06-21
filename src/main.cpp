#include "map.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  std::string input;
  std::getline(std::cin, input);
  Map mp(input);

  return 0;
}
