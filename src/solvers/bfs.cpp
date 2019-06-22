#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfsSolver(std::shared_ptr<Game> game) {
  while (true) {
    auto w = game->wrappers[0];
    const Trajectory traj = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    if (traj.path.size() == 0)
      break;

    const char c = Direction2Char(traj.path[0]);
    w->move(c);
    game->tick();
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs", bfsSolver);
