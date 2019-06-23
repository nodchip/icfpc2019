#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

using namespace std;

struct WrapperEngine {
  WrapperEngine(Game *game, int id) : m_game(game), m_id(id), w(game->wrappers[id].get()), m_num_manipulators(0) {};
  Wrapper *action() {
    if (m_game->num_boosters[BoosterType::MANIPULATOR] > 0) {
      if (m_num_manipulators % 2 == 0) {
        w->addManipulator(Point(1, 2 + m_num_manipulators / 2));
//        cout << m_id << ": add: " << m_num_manipulators << ", " << Point(1, 2 + m_num_manipulators / 2) << endl;
      } else {
        w->addManipulator(Point(1, - 2 - m_num_manipulators / 2));
//        cout << m_id << ": add: " << m_num_manipulators << ", " << Point(1, - 2 - m_num_manipulators / 2) << endl;
      }
      m_num_manipulators++;
    } else if (((m_game->map2d(w->pos) & CellType::kSpawnPointBit) != 0) && m_game->num_boosters[BoosterType::CLONING]) {
//      cout << m_id << ": clone: " << w->pos << ": " << m_game->num_boosters[BoosterType::CLONING] << endl;
      return w->cloneWrapper();
    } else {
    // dist1 check
      int p_max = 0;
       Direction d_max = Direction::W;
       w->move('W');
       const int w_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();
       if (w_paint > 2) {
         p_max = w_paint;
         d_max = Direction::W;
      }
      w->move('S');
      const int s_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();
      if (s_paint > 2) {
        p_max = s_paint;
        d_max = Direction::S;
      }
       w->move('A');
      const int a_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();
      if (a_paint > 2) {
        p_max = a_paint;
        d_max = Direction::A;
      }
      w->move('D');
       const int d_paint = (w->actions.back().absolute_new_wrapped_positions.size());
      w->undoAction();
      if (d_paint > 2) {
        p_max = d_paint;
        d_max = Direction::D;
      }
      if(p_max > 10) {
        const char c = Direction2Char(d_max);
        w->move(c);
//        cout << m_id << ": move undo: " << w->pos << ": " << c << endl;
      } else {
        const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*m_game, w->pos, DISTANCE_INF);
        if (trajs.size() == 0) {
          w->nop();
//          cout << m_id << ": nop:" << endl;
          return NULL;
        }
        const char c = Direction2Char(trajs[0].last_move);
        w->move(c);
//        cout << m_id << ": move: " << w->pos << ": " << c << endl;
      }
    }
    return NULL;
  }
  Game *m_game;
  int m_id;
  Wrapper *w;
  int m_num_manipulators;
};

std::string bfs5_plus_wipe_Solver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int num_wrappers = 1;
  vector<WrapperEngine> ws;
  ws.emplace_back(WrapperEngine(game, 0));
  int epoch(0);
  while (game->countUnWrapped() != 0) {
//    cout << epoch << ": ";
    epoch++;
    vector<int> cloned;
    for (auto &w : ws) {
      auto wc = w.action();
      if (wc != NULL) {
        cloned.emplace_back(num_wrappers);
        num_wrappers++;
      }
    }
    game->tick();
    displayAndWait(param, game);
    if (iter_callback && !iter_callback(game)) return game->getCommand();
    for (auto id : cloned) {
      ws.emplace_back(game, id);
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("bfs5_plus_wipe", bfs5_plus_wipe_Solver);
