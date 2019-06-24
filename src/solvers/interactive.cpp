#include <iostream>
#include <cctype>

#include "getch.h"
#include "solver_registry.h"

std::string interactiveSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int iter = 0;
  bool terminate = false;
  while (!terminate) {
    std::cout << "======= [iter: " << iter << ". wrappers = " << game->wrappers.size() << "] ========" << std::endl;
    std::cout << *game << std::endl;

    bool did_undo = false;
    for (int i = 0; !did_undo && !terminate && i < game->wrappers.size(); ++i) {
      while (true) {
        if (i == 0) {
          std::cout << "# " << i << ": Command [!]quit [U]undo [W/A/S/D/Z]move [E/Q]turn [F/L/R/C]boost [M]manipulate [T]teleport >" << std::flush;
        } else {
          std::cout << "# " << i << ": Command [!]quit [W/A/S/D/Z]move [E/Q]turn [F/L/R/C]boost [M]manipulate [T]teleport >" << std::flush;
        }
        Wrapper* w = game->wrappers[i].get();
#if defined(_MSC_VER)
#define getch _getch
#endif
        char c = getch();
        c = std::toupper(c);
        if (c == '!') {
          terminate = true;
          break;
        }
        if (c == 'W' || c == 'S' || c == 'A' || c == 'D') {
          if (w->isMoveable(c)) {
            w->move(c);
            break;
          } else {
            std::cout << "not moveable" << std::endl;
          }
        }
        if (c == 'Z') {
          w->nop();
          break;
        }
        if (c == 'E' || c == 'Q') {
          w->turn(c);
          break;
        }
        if (c == 'F' && game->num_boosters[BoosterType::FAST_WHEEL] > 0) { w->useBooster(c); break; }
        if (c == 'L' && game->num_boosters[BoosterType::DRILL] > 0) { w->useBooster(c); break; }
        if (c == 'R' && game->num_boosters[BoosterType::TELEPORT] > 0) { w->useBooster(c); break; }
        if (c == 'C' && game->num_boosters[BoosterType::CLONING] > 0 && (game->map2d(w->pos) & CellType::kSpawnPointBit) != 0) {
           w->cloneWrapper(); break;
        }
        if (c == 'M' && game->num_boosters[BoosterType::MANIPULATOR] > 0) {
          int x, y;
          std::cout << "(X, Y) >" << std::flush;
          std::cin >> x >> y;
          w->addManipulator({x, y});
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
        if (c == 'U' && game->time > 0 && i == 0) {
          // Undo the whole previous frame.
          game->undo();
          did_undo = true;
          break;
        }
        std::cout << "???" << std::endl;
      }
    }
    if (!terminate && !did_undo) {
      game->tick();
      if (!iter_callback(game)) return game->getCommand();
      ++iter;
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("interactive", interactiveSolver);
