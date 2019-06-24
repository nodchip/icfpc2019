#include <iostream>
#include <cctype>

#include "getch.h"
#include "solver_registry.h"

bool parseXY(std::string &command, std::vector<int> &indexs, int i, int &x, int &y) {
  indexs[i]++;
  bool minusX(false);
  x = 0;
  if (command[indexs[i]] == '-') {
    minusX = true;
    indexs[i]++;
  }
  while (command[indexs[i]] >= '0' && command[indexs[i]] <= '9') {
    x *= 10;
    x += (command[indexs[i]] - '0');
    indexs[i]++;
  }
  indexs[i]++;
  bool minusY(false);
  y = 0;
  if (command[indexs[i]] == '-') {
    minusY = true;
    indexs[i]++;
  }
  while (command[indexs[i]] >= '0' && command[indexs[i]] <= '9') {
    y *= 10;
    y += (command[indexs[i]] - '0');
    indexs[i]++;
  }
  indexs[i]++;
  if (minusX) x = -x;
  if (minusY) y = -y;
  return true;
}

std::string simulatorSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int iter = 0;
  bool terminate = false;
  std::string command;
  std::cin >> command;
  std::vector<std::string> commands;
  auto offset(std::string::size_type(0));
  while (true) {
    auto pos = command.find('#', offset);
    if (pos == std::string::npos) {
      commands.push_back(command.substr(offset));
      break;
    }
    commands.push_back(command.substr(offset, pos - offset));
    offset = pos + 1;
  }
  for (auto &c : commands) {
    std::cout << c << std::endl;
  }
  std::vector<int> indexs(commands.size());
  while (!terminate) {
    std::cout << "======= [iter: " << iter << ". wrappers = " << game->wrappers.size() << "] ========" << std::endl;
    std::cout << *game << std::endl;

    bool commandPresent(false);
    for (int i = 0; i < game->wrappers.size(); ++i) {
      while (true) {
        auto &command(commands[i]);
        Wrapper* w = game->wrappers[i].get();
        char c = 'Z';
        if (command.size() > indexs[i]) {
          c = command[indexs[i]];
          indexs[i]++;
          commandPresent = true;
        }
        std::cout << c << std::endl;
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
        if (c == 'R' && game->num_boosters[BoosterType::TELEPORT] > 0 && (game->map2d(w->pos) & CellType::kTeleportTargetBit) == 0) { w->useBooster(c); break; }
        if (c == 'C' && game->num_boosters[BoosterType::CLONING] > 0 && (game->map2d(w->pos) & CellType::kSpawnPointBit) != 0) {
           w->cloneWrapper(); break;
        }
        if (c == 'B' && game->num_boosters[BoosterType::MANIPULATOR] > 0) {
          int x, y;
          std::cout << "(X, Y) >" << std::flush;
          parseXY(command, indexs, i, x, y);
          std::cout << "(" << x << "," << y << ")\n";
          w->addManipulator({x, y});
          break;
        }
        if (c == 'T') {
          auto targets = enumerateCellsByMask(game->map2d, CellType::kTeleportTargetBit, CellType::kTeleportTargetBit);
          if (!targets.empty()) {
            std::cout << "targets:";
            for (auto t : targets) std::cout << t << " ";
            std::cout << ".";
            int x, y;
            std::cout << "(X, Y) >" << std::flush;
            parseXY(command, indexs, i, x, y);
            std::cout << "(" << x << "," << y << ")\n";
            if (game->map2d.isInside({x, y}) && (game->map2d({x, y}) & CellType::kTeleportTargetBit) != 0) {
              w->teleport({x, y});
              break;
            }
          }
        }
        std::cout << "???" << std::endl;
      }
    }
    if (commandPresent) {
      game->tick();
      if (!iter_callback(game)) return game->getCommand();
      ++iter;
    } else {
      terminate = true;
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("simulator", simulatorSolver);
