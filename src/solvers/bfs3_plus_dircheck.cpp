#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

std::string bfs3_plus_dircheck_Solver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int num_add_manipulators = 0;
  char lean = 'W';
  char antilean = 'S';
  char side = 'D';
  
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
      // std::cout<<*game<<std::endl;
      {
	w->move(lean);
	const int paint = (w->actions.back().absolute_new_wrapped_positions.size());
	w->undoAction();
	
	if (paint > 0){
	  w->move(lean);
	  side = lean;
	  if (lean == 'W'){
	    lean = 'A';
	    antilean = 'D';
	  }else if(lean == 'A'){
	    lean = 'S';
	    antilean = 'W';
	  }else if(lean == 'S'){
	    lean = 'D';
	    antilean = 'A';
	  }else{
	    lean = 'W';
	    antilean = 'S';
	  }
	  game->tick();
	  displayAndWait(param, game);
	  continue;
	}
      }
      {
	w->move(side);
	const int paint = (w->actions.back().absolute_new_wrapped_positions.size());
	w->undoAction();
	
	if (paint > 0){
	  w->move(side);
	  game->tick();
	  displayAndWait(param, game);
	  continue;
	}
      }
      {
	w->move(antilean);
	const int paint = (w->actions.back().absolute_new_wrapped_positions.size());
	w->undoAction();
	
	if (paint > 0){
	  w->move(antilean);
	  side = antilean;
	  if (lean == 'W'){
	    lean = 'D';
	    antilean = 'A';
	  }else if(lean == 'A'){
	    lean = 'W';
	    antilean = 'S';
	  }else if(lean == 'S'){
	    lean = 'A';
	    antilean = 'D';
	  }else{
	    lean = 'S';
	    antilean = 'W';
	  }

	  game->tick();
	  displayAndWait(param, game);
	  continue;
	}
	
      }
      
    }
    
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);
    int count = game->countUnWrapped();
    if (trajs.size() == 0)
      break;
    for(auto t : trajs){
      //std::cout<<t<<std::endl;
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

REGISTER_SOLVER("bfs3_plus_dircheck", bfs3_plus_dircheck_Solver);
