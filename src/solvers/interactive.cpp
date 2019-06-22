#include "../solver_registry.h"
#include <iostream>
#include <cctype>

#include "../getch.h"

std::string interactiveSolver(std::shared_ptr<Game> game) {
  int iter = 0;
  bool terminate = false;
  while (!terminate) {
    std::cout << "======= [iter: " << iter << "] ========" << std::endl;
    std::cout << *game << std::endl;

    bool did_undo = false;
    for (int i = 0; !terminate && i < game->wrappers.size(); ++i) {
      while (true) {
        std::cout << "# " << i << ": Command [!:quit] [U:undo] [W/A/S/D] [E/Q] [F/L/R/Z] [M:manipulate] [T:teleport]>" << std::flush;
        auto w = game->wrappers[i];
        char c = getch();
        c = std::toupper(c);
        if (c == '!') {
          terminate = true;
          break;
        }
        if (c == 'W' || c == 'S' || c == 'A' || c == 'D') {
          w->move(c);
          break;
        }
        if (c == 'E' || c == 'Q') {
          w->turn(c);
          break;
        }
        if (c == 'Z') {
          w->nop();
          break;
        }
        if (c == 'F' && game->fast_wheels > 0) { w->useBooster(c); break; }
        if (c == 'L' && game->drills > 0) { w->useBooster(c); break; }
        if (c == 'R' && game->teleports > 0) { w->useBooster(c); break; }
        if (c == 'C' && game->clonings > 0) { w->cloneWrapper(); break; }
        if (c == 'M' && game->num_manipulators > 0) {
          int x, y;
          std::cout << "(X, Y) >" << std::flush;
          std::cin >> x >> y;
          w->addManipulate({x, y});
          break;
        }
        if (c == 'T') {
          int x, y;
          std::cout << "(X, Y) >" << std::flush;
          std::cin >> x >> y;
          if (game->map2d.isInside({x, y}) && (game->map2d({x, y}) & CellType::kTeleportTargetBit) != 0) {
            w->teleport({x, y});
            break;
          }
        }
        if (c == 'U' && game->time > 0) {
          // first, undo [0, i)
          // then, undo the whole previous frame.
          for (int j = 0; j < i; ++j) {
            game->wrappers[j]->undoAction();
          }
          game->undo();
          did_undo = true;
          break;
        }
        std::cout << "???" << std::endl;
      }
    }
    if (!did_undo) {
      game->tick();
      ++iter;
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("interactive", interactiveSolver);