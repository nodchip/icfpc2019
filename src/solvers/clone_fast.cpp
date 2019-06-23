// based on bfs5_6.cpp but paint smaller area first.
#include <iostream>
#include <cctype>
#include <cmath>

#include "map_parse.h"
#include "solver_registry.h"
#include "solver_helper.h"

using namespace std;

namespace {
struct WrapperEngine {
  WrapperEngine(Game *game, int id) : m_game(game), m_id(id), m_wrapper(game->wrappers[id].get()), m_num_manipulators(0) { m_total_wrappers++; };
  Wrapper *action(double x, double y, std::vector<Trajectory> &to_go) {
    //cout<<to_go.size()<<","<<(m_wrapper->pos.x)<<","<<(m_wrapper->pos.y)<<std::endl;
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
    } else if(to_go.size()!=0){

      /*
      for(auto tg : to_go){
	cout<<tg<<" ";
      }
      cout<<endl;
      */
      m_wrapper->move(Direction2Char(to_go[0].last_move));
      to_go.erase(to_go.begin());
    }else {
      auto pos = m_wrapper->pos;
      x -= pos.x;
      y -= pos.y;
      char c = ' ';
      /*
      static const int kMask = CellType::kObstacleBit | CellType::kWrappedBit;
      if (abs(x) > abs(y)) {
        if (x > 0) {
          if ((m_game->map2d(pos + Point(1, 0)) & kMask) == 0) {
            c = 'D';
          } else if (y > 0) {
            if ((m_game->map2d(pos + Point(0, 1)) & kMask) == 0) {
              c = 'W';
            }
          } else if (y < 0) {
            if ((m_game->map2d(pos + Point(0, -1)) & kMask) == 0) {
              c = 'S';
            }
          }
        } else if (x < 0) {
          if ((m_game->map2d(pos + Point(-1, 0)) & kMask) == 0) {
            c = 'A';
          } else if (y > 0) {
            if ((m_game->map2d(pos + Point(0, 1)) & kMask) == 0) {
              c = 'W';
            }
          } else if (y < 0) {
            if ((m_game->map2d(pos + Point(0, -1)) & kMask) == 0) {
              c = 'S';
            }
          }
        }
      } else {
        if (y > 0) {
          if ((m_game->map2d(pos + Point(0, 1)) & kMask) == 0) {
            c = 'W';
          } else if (x > 0) {
            if ((m_game->map2d(pos + Point(1, 0)) & kMask) == 0) {
              c = 'D';
            }
          } else if (x < 0) {
            if ((m_game->map2d(pos + Point(-1, 0)) & kMask) == 0) {
              c = 'A';
            }
          }
        } else if (y < 0) {
          if ((m_game->map2d(pos + Point(0, -1)) & kMask) == 0) {
            c = 'S';
          } else if (x > 0) {
            if ((m_game->map2d(pos + Point(1, 0)) & kMask) == 0) {
              c = 'D';
            }
          } else if (x < 0) {
            if ((m_game->map2d(pos + Point(-1, 0)) & kMask) == 0) {
              c = 'A';
            }
          }
        }
      }
      */
      if (c != ' ') {
        m_wrapper->move(c);
        //cout << m_id << ": nmove: " << c << endl;
      } else {
        std::vector<Trajectory> trajs;

        auto ccs = disjointConnectedComponentsByMask(m_game->map2d, 0b11, 0b00);
        if (ccs.size() > 1 && m_id == 0) {
          // 孤立領域があれば最小のものに向かう
          std::sort(ccs.begin(), ccs.end(), [](const auto& lhs, const auto& rhs) { return lhs.size() < rhs.size(); });
          auto small_cc = ccs.front();
          assert (!small_cc.empty());
          auto target = small_cc.front();
        
          trajs = map_parse::findTrajectory(*m_game, m_wrapper->pos, target, DISTANCE_INF, false, false);
        } else {
          // なければbfs5_6と同じ
          trajs = map_parse::findNearestUnwrapped(*m_game, m_wrapper->pos, DISTANCE_INF, false, false);
        }
        if (trajs.size() == 0) {
          m_wrapper->nop();
          return NULL;
        }
        c = Direction2Char(trajs[0].last_move);
        m_wrapper->move(c);
        //cout << m_id << "traj : move: " << c << endl;
      }
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

std::vector<std::vector<Trajectory>> getItemMatrixfast(Game* game, const int mask){
  std::vector<std::vector<Trajectory>> output;
  const int wsize = game->wrappers.size();
  output.resize(wsize);
  // wrapper0以外はアイテムを明示的に探さない！！
  for(int j=0;j<1;++j){
    std::vector<Trajectory> trajs = map_parse::findNearestByBit(*game, game->wrappers[j]->pos, DISTANCE_INF, mask);
    output[j] = trajs;
  }
  return output;
}

std::string clonefastSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int num_wrappers = 1;
  vector<WrapperEngine> ws;
  bool clone_mode = false;
  ws.emplace_back(WrapperEngine(game, 0));
  int epoch(0);
  bool clone_exist = (enumerateCellsByMask(game->map2d, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit).size() > 0);
  std::vector<std::vector<Trajectory>> cmat;
  cmat = std::vector<std::vector<Trajectory>>(game->wrappers.size());
  
  if(clone_exist){
    //cout<<"clone found"<<endl;
  }
  while (!game->isEnd()) {
//    cout << epoch << ": ";
    //cout<<*game<<endl;
    clone_exist = (enumerateCellsByMask(game->map2d, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit).size() > 0);

    if(cmat.size() < game->wrappers.size()){
      cmat.resize(game->wrappers.size());
    }
      
    if(clone_mode && game->num_boosters[BoosterType::CLONING] > 0){
      clone_mode = false;
      //cout<<"route gen for spawn"<<endl;
      cmat = getItemMatrixfast(game,  CellType::kSpawnPointBit);
    }else if(!clone_mode && cmat[0].size() == 0 && clone_exist){
      //cout<<"route gen for clone"<<endl;
      clone_mode = true;
      cmat = getItemMatrixfast(game, CellType::kBoosterCloningBit);
    }
    
    epoch++;
    vector<int> cloned;
    double x(0.0), y(0.0);
    for (auto &w : game->wrappers) {
      x += w->pos.x;
      y += w->pos.y;
    }
    x /= ws.size();
    y /= ws.size();

    int ws_itr = 0;
    for (auto &w : ws) {
      auto wc = w.action(x, y, cmat[ws_itr]);
      ws_itr += 1;
      //auto wc = w.action(x, y, std::vector<Trajectory>(0));
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

REGISTER_SOLVER("clone_fast", clonefastSolver);
