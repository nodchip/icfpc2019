#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfsSolver(SolverParam param, Game* game) {
  while (true) {
    Wrapper* w = game->wrappers[0].get();
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    if (trajs.size() == 0)
      break;
    for(auto t : trajs){
      const char c = Direction2Char(t.last_move);
      w->move(c);
      game->tick();
      displayAndWait(param, game);
    }
  }
  std::cout<<*game<<std::endl;
  return game->getCommand();
}

REGISTER_SOLVER("bfs", bfsSolver);
