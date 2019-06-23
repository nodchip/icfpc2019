// based on bfs5_6.cpp but paint smaller area first.
#include <iostream>
#include <limits>
#include <cctype>
#include <cmath>

#include "map_parse.h"
#include "solver_registry.h"
#include "solver_helper.h"

using namespace std;

// clone_fastが雛形。近くにあるアイテムを拾うようにする。

namespace {

struct WrapperEngine {
  WrapperEngine(Game *game, int id) : m_game(game), m_id(id), m_wrapper(game->wrappers[id].get()), m_num_manipulators(0) { m_total_wrappers++; };
  Wrapper *action(double x, double y, std::vector<Trajectory> &to_go, ConnectedComponentAssignmentForParanoid& cc_assignment) {
    if (m_game->num_boosters[BoosterType::MANIPULATOR] > 0 && (m_game->num_boosters[BoosterType::MANIPULATOR] + m_total_manipulators > m_total_wrappers * m_num_manipulators)) {
      if (m_num_manipulators % 2 == 0) {
        m_wrapper->addManipulator(Point(0, 1 + m_num_manipulators / 2));
      } else {
        m_wrapper->addManipulator(Point(0, - 1 - m_num_manipulators / 2));
      }
      m_num_manipulators++;
      m_total_manipulators++;
    } else if (((m_game->map2d(m_wrapper->pos) & CellType::kSpawnPointBit) != 0) && m_game->num_boosters[BoosterType::CLONING]) {
      return m_wrapper->cloneWrapper();
    } else if(to_go.size()!=0){
      m_wrapper->move(Direction2Char(to_go[0].last_move));
      to_go.erase(to_go.begin());
    }else {
      auto pos = m_wrapper->pos;
      std::vector<Trajectory> trajs;

      cc_assignment.update();
      if (cc_assignment.hasDisjointComponents() && cc_assignment.isComponentAssignedToWrapper(m_id)) {
        Trajectory t;
        if (pointToDirection(t.last_move, cc_assignment.getSuggestedMotionOfWrapper(m_id))) {
          trajs = { t };
        }
        /*
        // 割り当てられた領域に向かう
        auto target = cc_assignment.getTargetOfWrapper(m_id);
        trajs = map_parse::findTrajectory(*m_game, pos, target, DISTANCE_INF, false, false);
        // ただし密集した孤立領域に遠くから集まるのは効率が悪いので、近い孤立領域のみ。
        if (trajs.size() > 100) {
          trajs.clear();
        }
        */
      }
      if (trajs.empty()) {
        // なければbfs5_6と同じ
        trajs = map_parse::findNearestUnwrapped(*m_game, pos, DISTANCE_INF, false, false);
      }
      if (trajs.size() == 0) {
        m_wrapper->nop();
        return NULL;
      }
      char c = Direction2Char(trajs[0].last_move);
      m_wrapper->move(c);
      //cout << m_id << "traj : move: " << c << endl;
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

static std::vector<std::vector<Trajectory>> getItemMatrixpick(Game* game, const int mask, const int max_dist = DISTANCE_INF, bool onlyzero = true){
  std::vector<std::vector<Trajectory>> output;
  const int wsize = game->wrappers.size();
  output.resize(wsize);
  // wrapper0以外はアイテムを明示的に探さない！！
  for(int j=0;j<wsize;++j){
    if(onlyzero && j!=0){
      break;
    }
    std::vector<Trajectory> trajs = map_parse::findNearestByBit(*game, game->wrappers[j]->pos, max_dist, mask);
    output[j] = trajs;
  }
  return output;
}

std::string pickStrictParanoidsSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  int num_wrappers = 1;
  vector<WrapperEngine> ws;
  bool clone_mode = false;
  ws.emplace_back(WrapperEngine(game, 0));
  int epoch(0);
  bool clone_exist = (enumerateCellsByMask(game->map2d, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit).size() > 0);
  std::vector<std::vector<Trajectory>> cmat;
  cmat = std::vector<std::vector<Trajectory>>(game->wrappers.size());

  ConnectedComponentAssignmentForParanoid cc_assignment(game, 10);

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
      cmat = getItemMatrixpick(game,  CellType::kSpawnPointBit);
    }else if(!clone_mode && cmat[0].size() == 0 && clone_exist){
      //cout<<"route gen for clone"<<endl;
      clone_mode = true;
      cmat = getItemMatrixpick(game, CellType::kBoosterCloningBit);
    }


    // neighbor item search
    {
      std::vector<std::vector<Trajectory>> bmat;
      bmat = getItemMatrixpick(game, CellType::kBoosterManipulatorBit, 3, false);
      for(int i=0; i<game->wrappers.size();++i){
        if(bmat[i].size()!=0 && cmat[i].size()==0){
          /*
          cout<<"wrapper "<<i<<" item traj"<<endl;
          for(auto tj : bmat[i]){
            cout<<tj<<" ";
          }
          cout<<endl;
          */
          cmat[i] = bmat[i]; // push traj to get item
        }
      }
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

    // delayed assign component - wrapper.
    cc_assignment.delayUpdate();

    int ws_itr = 0;
    for (auto &w : ws) {
      auto wc = w.action(x, y, cmat[ws_itr], cc_assignment);
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

REGISTER_SOLVER("pick_strict_paranoids", pickStrictParanoidsSolver);
