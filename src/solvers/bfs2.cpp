#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfsSolver2(SolverParam param, Game::Ptr game) {
  while (true) {
    auto w = game->wrappers[0];
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    /*
    for(auto t : trajs){
      std::cout<<t<<" ";
    }
    std::cout<<std::endl;
    */
    if (trajs.size() == 0)
      break;
    const char c = Direction2Char(trajs[0].last_move);
    w->move(c);
    game->tick();
    displayAndWait(param, game);
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs2", bfsSolver2);
