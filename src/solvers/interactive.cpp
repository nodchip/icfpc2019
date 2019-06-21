#include "../solver_registry.h"
#include <iostream>
#include <cctype>

#include "../getch.h"

std::string interactiveSolver(Game game) {
  int iter = 0;
  while (true) {
    std::cout << "======= [iter: " << iter << "] ========" << std::endl;
    std::cout << game << std::endl;
    std::cout << "Command [!:quit] [W/A/S/D] [E/Q] [F/L/R/Z] [M:manipulate] [T:teleport]>" << std::flush;
    char c = getch();
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
    if (c == 'F' && game.fast_wheels > 0) { game.useBooster(c); }
    if (c == 'L' && game.drills > 0) { game.useBooster(c); }
    if (c == 'R' && game.teleports > 0) { game.useBooster(c); }
    if (c == 'Z') {
      game.nop();
    }
    if (c == 'M' && game.num_manipulators > 0) {
      int x, y;
      std::cout << "(X, Y) >" << std::flush;
      std::cin >> x >> y;
      game.addManipulate({x, y});
    }
    if (c == 'T') {
      int x, y;
      std::cout << "(X, Y) >" << std::flush;
      std::cin >> x >> y;
      if (game.map2d.isInside({x, y}) && (game.map2d({x, y}) & CellType::kTeleportTargetBit) != 0) {
        game.teleport({x, y});
      }
    }
    ++iter;
  }
  return game.getCommand();
}

REGISTER_SOLVER("interactive", interactiveSolver);