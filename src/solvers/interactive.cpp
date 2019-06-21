#include "../solver_registry.h"
#include <iostream>
#include <cctype>

std::string interactiveSolver(Game game) {
  int iter = 0;
  while (true) {
    std::cout << "======= [iter: " << iter << "] ========" << std::endl;
    std::cout << game << std::endl;
    std::cout << "Command [!] [W/A/S/D] [E/Q] [F/L/R/Z] [M:manipulate] [T:teleport]>" << std::flush;
    char c;
    std::cin >> c;
    c = std::toupper(c);
    if (c == '!') {
      break;
    }
    if (c == 'W' || c == 'S' || c == 'A' || c == 'D') {
      game.move(c);
    }
    if (c == 'E' || c == 'Q') {
      game.turn(c);
    }
    if (c == 'F' || c == 'L' || c == 'R') {
      game.useBooster(c);
    }
    if (c == 'Z') {
      game.nop();
    }
    if (c == 'M') {
      int x, y;
      std::cout << "(X, Y) >" << std::flush;
      std::cin >> x >> y;
      game.addManipulate({x, y});
    }
    if (c == 'T') {
      int x, y;
      std::cout << "(X, Y) >" << std::flush;
      std::cin >> x >> y;
      game.teleport({x, y});
    }
    ++iter;
  }
  return game.getCommand();
}

REGISTER_SOLVER("interactive", interactiveSolver);