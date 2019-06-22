#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfs3Solver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int num_add_manipulators = 0;
  while (true) {
    Wrapper* w = game->wrappers[0].get();
    if (game->num_boosters[BoosterType::MANIPULATOR] > 0) {
      if (num_add_manipulators % 2 == 0) {
        w->addManipulate(Point(1, 2 + num_add_manipulators / 2));
      } else {
        w->addManipulate(Point(1, - 2 - num_add_manipulators / 2));
      }
      game->tick();
      displayAndWait(param, game);
      if (iter_callback && !iter_callback(game)) return game->getCommand();
      num_add_manipulators++;
    }
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    int count = game->countUnWrapped();
    if (trajs.size() == 0)
      break;
    for(auto t : trajs){
      const char c = Direction2Char(t.last_move);
      w->move(c);
      game->tick();
      displayAndWait(param, game);
      if (iter_callback && !iter_callback(game)) return game->getCommand();
      if (count != game->countUnWrapped()) {
        break;
      }
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs3", bfs3Solver);
