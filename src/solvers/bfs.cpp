#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfsSolver(std::shared_ptr<Game> game) {
  while (true) {
    auto w = game->wrappers[0];
    const Trajectory traj = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    std::cout<<traj.path.size()<<std::endl;
    if (traj.path.size() == 0)
      break;
    for(auto p : traj.path){
      const char c = Direction2Char(p);
      w->move(c);
      game->tick();
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs", bfsSolver);
