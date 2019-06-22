#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfs2Solver(SolverParam param, Game::Ptr game) {
  while (true) {
    auto w = game->wrappers[0];
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    /*
    for(auto t : trajs){
      std::cout<<t<<" ";
    }
    std::cout<<std::endl;
    */
    int count = game->countUnWrapped();
    if (trajs.size() == 0)
      break;
    for(auto t : trajs){
      const char c = Direction2Char(t.last_move);
      w->move(c);
      game->tick();
      displayAndWait(param, game);
      if (count != game->countUnWrapped()) {
        break;
      }
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs2", bfs2Solver);