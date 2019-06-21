#include <iostream>
#include <vector>
#include <string>

#include "map.h"
#include "wrappy.h"

int main(int argc, char* argv[]) {

  // test input: part-1-example/example-01.map
  // TODO(peria): Make Map parse .desc well.
  std::vector<std::string> input;
  for (std::string l; std::getline(std::cin, l);)
    input.emplace_back(l);
  Map mp(input);
  Wrappy wrappy(mp);

  // Do something with using |wrappy|

  // Test output
  std::cout << wrappy << "\n";

  return 0;
}
