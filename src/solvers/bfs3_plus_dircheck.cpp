#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"



std::string bfs3_plus_dircheck_Solver(SolverParam param, Game::Ptr game) {
  int num_add_manipulators = 0;
  Direction prev_dir = Direction::D;
  while (true) {
    auto w = game->wrappers[0];
    if (game->num_boosters[BoosterType::MANIPULATOR] > 0) {
      if (num_add_manipulators % 2 == 0) {
//        w->addManipulate(Point())
        w->addManipulate(Point(1, 2 + num_add_manipulators / 2));
      } else {
//        w->addManipulate();
        w->addManipulate(Point(1, - 2 - num_add_manipulators / 2));
      }
      game->tick();
      displayAndWait(param, game);
      num_add_manipulators++;
    }

    // dist1 check
    {
      int fill_w = 0;
      if (game->map2d(w->pos.x, w->pos.y+1) & CellType::kObstacleBit == 0){
	if(game->map2d(w->pos.x, w->pos.y+1) & CellType::kWrappedBit == 0){
	  fill_w += 1;
	}
	for(auto& mp : w->manipulators){
	  
	}
      }
      
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
      if (count != game->countUnWrapped()) {
        break;
      }
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs3_plus_dircheck", bfs3_plus_dircheck_Solver);
