#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfs3_plus_wipe_Solver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int num_add_manipulators = 0;
  while (true) {
    Wrapper* w = game->wrappers[0].get();
    if (game->num_boosters[BoosterType::MANIPULATOR] > 0) {
      if (num_add_manipulators % 2 == 0) {
        w->addManipulator(Point(1, 2 + num_add_manipulators / 2));
      } else {
        w->addManipulator(Point(1, - 2 - num_add_manipulators / 2));
      }
      game->tick();
      displayAndWait(param, game);
      if (iter_callback && !iter_callback(game)) return game->getCommand();
      num_add_manipulators++;
    }

    // dist1 check
    {
      int p_max = 0;
      Direction d_max = Direction::W;
      
      w->move('W');
      const int w_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();

      if (w_paint > 2){
	p_max = w_paint;
	d_max = Direction::W;
      }

      w->move('S');
      const int s_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();

      if (s_paint > 2){
	p_max = s_paint;
	d_max = Direction::S;
      }

      w->move('A');
      const int a_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();

      if (a_paint > 2){
	p_max = a_paint;
	d_max = Direction::A;
      }

      w->move('D');
      const int d_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();
      
      if (d_paint > 2){
	p_max = d_paint;
	d_max = Direction::D;
      }
      /*
      std::cout<<*game<<std::endl;
      std::cout<<w_paint<<","<<s_paint<<","<<a_paint<<","<<d_paint<<std::endl;
      */
      if(p_max > 10){
	const char c = Direction2Char(d_max);
	w->move(c);
	game->tick();
	displayAndWait(param, game);
  if (iter_callback && !iter_callback(game)) return game->getCommand();
	continue;
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
      if (iter_callback && !iter_callback(game)) return game->getCommand();
      if (count != game->countUnWrapped()) {
        break;
      }
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs3_plus_wipe", bfs3_plus_wipe_Solver);
