// based on bfs5_6.cpp but paint smaller area first.
#include <iostream>
#include <cctype>
#include <cmath>

#include "map_parse.h"
#include "solver_registry.h"
#include "solver_helper.h"
#include "solver_utils.h"

//#define DEBUG_PRINT

using namespace std;

namespace {
struct WrapperEngine {
  WrapperEngine(Game *game, int id) : m_game(game), m_id(id), m_dir(-1), m_wrapper(game->wrappers[id].get()), m_num_manipulators(0) { m_total_wrappers++; };
  Wrapper *action(double &x, double &y, vector<vector<double>> &evalc) {
    if (m_game->num_boosters[BoosterType::MANIPULATOR] > 0 && (m_game->num_boosters[BoosterType::MANIPULATOR] + m_total_manipulators > m_total_wrappers * m_num_manipulators)) {
      if (m_num_manipulators % 2 == 0) {
        m_wrapper->addManipulator(Point(0, 1 + m_num_manipulators / 2));
//        cout << m_id << ": add: " << m_num_manipulators << ", " << Point(1, 2 + m_num_manipulators / 2) << endl;
      } else {
        m_wrapper->addManipulator(Point(0, - 1 - m_num_manipulators / 2));
//        cout << m_id << ": add: " << m_num_manipulators << ", " << Point(1, - 2 - m_num_manipulators / 2) << endl;
      }
      m_num_manipulators++;
      m_total_manipulators++;
    } else if (((m_game->map2d(m_wrapper->pos) & CellType::kSpawnPointBit) != 0) && m_game->num_boosters[BoosterType::CLONING]) {
//      cout << m_id << ": clone: " << m_wrapper->pos << endl;
      return m_wrapper->cloneWrapper();
    } else {
      auto pos = m_wrapper->pos;
#ifdef DEBUG_PRINT
      cout << m_id << ": " << pos << endl;
#endif
      Point d[4] = {Point(0, 1), Point(0, -1), Point(1, 0), Point(-1, 0)};
      char chd[4] = {'W', 'S', 'D', 'A'};
      double meval(0.0);
      int mdir(-1);
      for (int i = 0; i < 4; ++i) {
        auto ep = pos + d[i];
        if (m_game->map2d.isInside(ep)) {
#ifdef DEBUG_PRINT
          cout << d[i] << ": " << evalc[ep.y][ep.x] << endl;
#endif
          if (meval < evalc[ep.y][ep.x]) {
            meval = evalc[ep.y][ep.x];
            mdir = i;
          }
        }
      }
      if (mdir >= 0) {
        m_wrapper->move(chd[mdir]);
        m_dir = mdir;
      } else {
        std::vector<Trajectory> trajs;

        auto ccs = disjointConnectedComponentsByMask(m_game->map2d, 0b11, 0b00);
        if (ccs.size() > 1) {
          // 孤立領域があれば最小のものに向かう
          std::sort(ccs.begin(), ccs.end(), [](const auto& lhs, const auto& rhs) { return lhs.size() < rhs.size(); });
          auto small_cc = ccs.front();
          assert (!small_cc.empty());
          auto target = small_cc.front();
        
          trajs = map_parse::findTrajectory(*m_game, m_wrapper->pos, target, DISTANCE_INF);
        } else {
          // なければbfs5_6と同じ
          trajs = map_parse::findNearestUnwrapped(*m_game, m_wrapper->pos, DISTANCE_INF);
        }
        if (trajs.size() == 0) {
          m_wrapper->nop();
          return NULL;
        }
        char c = Direction2Char(trajs[0].last_move);
        m_wrapper->move(c);
  //      cout << m_id << ": move: " << c << endl;
      }
    }
    return NULL;
  }
  Game *m_game;
  int m_id;
  int m_dir;
  Wrapper *m_wrapper;
  int m_num_manipulators;
  static int m_total_manipulators;
  static int m_total_wrappers;
};

int WrapperEngine::m_total_manipulators = 0;
int WrapperEngine::m_total_wrappers = 0;
};

std::string evaluateSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  auto eval(utils::getGloryMap(*game));
  auto evalc(eval);
#ifdef DEBUG_PRINT
  for (const auto &l : eval) {
    for (const auto &e : l) {
      cout << e << ", ";
    }
    cout << endl;
  }
#endif
  int num_wrappers = 1;
  vector<WrapperEngine> ws;
  ws.emplace_back(WrapperEngine(game, 0));
  int epoch(0);
  while (!game->isEnd()) {
    utils::processCurrentGloryMap(*game, eval, evalc);
#ifdef DEBUG_PRINT
    cout << game->time << endl;
    for (const auto &l : evalc) {
      for (const auto &e : l) {
        cout << e << ", ";
      }
      cout << endl;
    }
#endif
//    cout << epoch << ": ";
    epoch++;
    vector<int> cloned;
    double x(0.0), y(0.0);
    for (auto &w : game->wrappers) {
      x += w->pos.x;
      y += w->pos.y;
    }
    x /= ws.size();
    y /= ws.size();
    for (auto &w : ws) {
      auto wc = w.action(x, y, evalc);
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

REGISTER_SOLVER("evaluate", evaluateSolver);
