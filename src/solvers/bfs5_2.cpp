#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"

using namespace std;

namespace {
struct WrapperEngine {
  WrapperEngine(Game *game, int id) : m_game(game), m_id(id), m_wrapper(game->wrappers[id].get()), m_num_manipulators(0) { m_total_wrappers++; };
  Wrapper *action() {
    if (m_game->num_boosters[BoosterType::MANIPULATOR] > 0 && (m_total_manipulators > m_total_wrappers * m_num_manipulators)) {
      if (m_num_manipulators % 2 == 0) {
        m_wrapper->addManipulate(Point(1, 2 + m_num_manipulators / 2));
        cout << m_id << ": add: " << m_num_manipulators << ", " << Point(1, 2 + m_num_manipulators / 2) << endl;
      } else {
        m_wrapper->addManipulate(Point(1, - 2 - m_num_manipulators / 2));
        cout << m_id << ": add: " << m_num_manipulators << ", " << Point(1, - 2 - m_num_manipulators / 2) << endl;
      }
      m_num_manipulators++;
      m_total_manipulators++;
    } else if (((m_game->map2d(m_wrapper->pos) & CellType::kSpawnPointBit) != 0) && m_game->num_boosters[BoosterType::CLONING]) {
//      cout << m_id << ": clone: " << m_wrapper->pos << endl;
      return m_wrapper->cloneWrapper();
    } else {
      const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*m_game, m_wrapper->pos, DISTANCE_INF);
      if (trajs.size() == 0) {
        m_wrapper->nop();
        return NULL;
      }
      const char c = Direction2Char(trajs[0].last_move);
      m_wrapper->move(c);
//      cout << m_id << ": move: " << c << endl;
    }
    return NULL;
  }
  Game *m_game;
  int m_id;
  Wrapper *m_wrapper;
  int m_num_manipulators;
  static int m_total_manipulators;
  static int m_total_wrappers;
};

int WrapperEngine::m_total_manipulators = 0;
int WrapperEngine::m_total_wrappers = 0;
};

std::string bfs5_2Solver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
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

REGISTER_SOLVER("bfs5_2", bfs5_2Solver);
