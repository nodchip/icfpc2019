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
  WrapperEngine(Game *game, int id, int iter) : m_game(game), m_id(id), m_wrapper(game->wrappers[id].get()), m_num_manipulators(0) { 
    m_dstart = rand() % 2 == 0;
    m_astart = rand() % 2 == 0;
    m_total_wrappers++; 
  }
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
        trajs = map_parse::findNearestUnwrapped(*m_game, pos, DISTANCE_INF, m_dstart, m_astart);
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
  bool m_dstart = false;
  bool m_astart = false;
  static int m_total_manipulators;
  static int m_total_wrappers;
};

int WrapperEngine::m_total_manipulators = 0;
int WrapperEngine::m_total_wrappers = 0;
};

static std::vector<std::vector<Trajectory>> getItemMatrixpick(Game* game, std::vector<WrapperEngine>& ws, const int mask, const int max_dist = DISTANCE_INF, bool onlyzero = true){
  std::vector<std::vector<Trajectory>> output;
  const int wsize = game->wrappers.size();
  output.resize(wsize);
  // wrapper0以外はアイテムを明示的に探さない！！
  for(int j=0;j<wsize;++j){
    if(onlyzero && j!=0){
      break;
    }
    std::vector<Trajectory> trajs = map_parse::findNearestByBit(*game, game->wrappers[j]->pos, max_dist, mask, ws[j].m_dstart, ws[j].m_astart);
    output[j] = trajs;
  }
  return output;
}

static bool no_spawner(Game* game, const std::vector<std::vector<Trajectory>> &cmat){

  for(auto &cm : cmat){
    if(cm.size()>0){
      const Point pt = cm[cm.size()-1].pos;
      if(game->map2d(pt.x, pt.y) & CellType::kSpawnPointBit){
	return false;
      }
    }
  }

  const int wsize = game->wrappers.size();
  for(int j=0;j<wsize;++j){
    const Point pt = game->wrappers[j]->pos;
    if(game->map2d(pt.x, pt.y) & CellType::kSpawnPointBit){
      return false;
    } 
  }
  
  return true;
}

static std::vector<Trajectory> getTrajClone(Game* game, std::vector<WrapperEngine>& ws, const int wid, const std::vector<std::vector<Trajectory>> &cmat, const int max_dist){
  
  // 他のwrapperの計画を見た上で割り振るジョブをキメる
  
  // 取得予約済みのアイテムの種類と場所
  std::vector<int> res_item;
  std::vector<Point> res_point;

  // 取りに行くアイテム
  int target_item = CellType::kBoosterCloningBit;
  bool spawner = false;
  int clone_cnt = 0;
  for(auto &cm : cmat){
    if(cm.size()>0){
      const Point pt = cm[cm.size()-1].pos;
      res_item.push_back(game->map2d(pt.x, pt.y));
      if(game->map2d(pt.x, pt.y) & CellType::kSpawnPointBit){
	spawner = true;
      }
      if(game->map2d(pt.x, pt.y) & CellType::kBoosterCloningBit){
	clone_cnt += 1;
      }
      res_point.push_back(pt);
    }
  }

  if(clone_cnt == (enumerateCellsByMask(game->map2d, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit).size())){
    return std::vector<Trajectory>(0);
  }
    
  const int wsize = game->wrappers.size();
  for(int j=0;j<wsize;++j){
    const Point pt = game->wrappers[j]->pos;
    if(game->map2d(pt.x, pt.y) & CellType::kSpawnPointBit){
      spawner = true;
    } 
  }
  
  if(!spawner && game->num_boosters[BoosterType::CLONING] > 0){
    cout<<"no spawner found"<<endl;
    target_item = CellType::kSpawnPointBit;
  }


  for(int i=0;i<res_point.size();++i){
    // 取得済みアイテムを消した上で探索(とても危ない)
    game->map2d(res_point[i].x, res_point[i].y) = CellType::kWrappedBit;
  }
  
  std::vector<Trajectory> trajs = map_parse::findNearestByBit(*game, game->wrappers[wid]->pos, max_dist, target_item, ws[wid].m_dstart, ws[wid].m_astart);

  for(int i=0;i<res_point.size();++i){
    // 取得済みアイテムを戻す(とても危ない)
    game->map2d(res_point[i].x, res_point[i].y) = res_item[i];
  }

  // debug
  /*
  for(auto traj : trajs){
    cout<<traj<<" ";
  }
  cout<<endl;
  */

  if(trajs.size()>0){
    cout<<"multispawn "<<wid<<","<<trajs.size()<<endl;
  }
  return trajs;
}


std::string multispawnSolverSub(SolverParam param, Game* game, SolverIterCallback iter_callback, int iter) {
  int num_wrappers = 1;
  vector<WrapperEngine> ws;
  
  bool clone_mode = (game->num_boosters[BoosterType::CLONING] > 0);
  ws.emplace_back(WrapperEngine(game, 0, iter));
  
  int epoch(0);
  bool clone_exist = (enumerateCellsByMask(game->map2d, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit).size() > 0);
  std::vector<std::vector<Trajectory>> cmat;
  cmat = std::vector<std::vector<Trajectory>>(game->wrappers.size());

  ConnectedComponentAssignmentForParanoid cc_assignment(game,
    10 /* distance_threshold */,
    100 /* small_region_bonus */);

  if(clone_exist){
    //cout<<"clone found"<<endl;
  }
  while (!game->isEnd()) {
    game->clearDebugKeyValues();
    game->addDebugKeyValue("clone_exist", clone_exist);
    game->addDebugKeyValue("clone_mode", clone_mode);

//    cout << epoch << ": ";
    //cout<<*game<<endl;
    clone_exist = (enumerateCellsByMask(game->map2d, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit).size() > 0);

    if(cmat.size() < game->wrappers.size()){
      cmat.resize(game->wrappers.size());
    }

    for(int i=0; i<game->wrappers.size();++i){
      //cout<<cmat[i].size()<<endl;
      if(clone_exist && cmat[i].size()==0){
	//cout<<"trajclone"<<endl;
	cmat[i] = getTrajClone(game, ws, i, cmat, DISTANCE_INF);
      }else if(game->num_boosters[BoosterType::CLONING] > 0 && no_spawner(game, cmat) && cmat.size()==1){
	// spawnができるのにspawnしようとしている駒が居ない場合、wrapperのtrajを上書きする
	cout<<"check no spawner"<<endl;
	cmat[i] = getTrajClone(game, ws, i, cmat, DISTANCE_INF);
      }
    }
    // neighbor item search
    {
      std::vector<std::vector<Trajectory>> bmat;
      bmat = getItemMatrixpick(game, ws, CellType::kBoosterManipulatorBit, 3, false);
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

    game->addDebugKeyValue("w0_has_tgt", cmat[0].size() != 0);
    //game->addDebugKeyValue("n_cc", disjointConnectedComponentsByMask(game->map2d,
    //  CellType::kObstacleBit | CellType::kWrappedBit, 0).size());
    
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
      ws.emplace_back(game, id, iter);
    }
  }

  return game->getCommand();
}

std::string multispawnSolver(SolverParam param, Game* game_org, SolverIterCallback iter_callback) {
  srand(3333);

  std::unique_ptr<Game> best_game;
  int best_time = std::numeric_limits<int>::max();

  for (int iter = 0; iter < 2; ++iter) {
    auto copied_game = std::make_unique<Game>(*game_org);
    //std::cerr << iter << " " << copied_game->wrappers.size() << std::endl;
    multispawnSolverSub(param, copied_game.get(), iter_callback, iter);
    if (copied_game->isEnd()) {
      //std::cerr << "ITER " << iter << " => " << copied_game->time << std::endl;
      //std::cerr << "Command: " << copied_game->getCommand() << std::endl;
      //std::cerr << iter << " " << copied_game->wrappers.size() << std::endl;
      if (copied_game->time < best_time) {
        best_time = copied_game->time;
        best_game = std::move(copied_game);
      }
    }
  }

  if (best_game) {
    *game_org = *best_game.get();
    //std::cerr << "Command: " << game_org->getCommand() << std::endl;
    //std::cerr << game_org->wrappers.size() << std::endl;
  }
  return game_org->getCommand();
}

REGISTER_SOLVER("multispawn", multispawnSolver);
