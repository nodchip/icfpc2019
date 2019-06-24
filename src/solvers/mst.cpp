// based on bfs5_6.cpp but paint smaller area first.
#include <iostream>
#include <limits>
#include <cctype>
#include <cmath>
#include <queue>
#include <random>

#include "map_parse.h"
#include "solver_registry.h"
#include "solver_helper.h"

#pragma optimize( "", off )

using namespace std;

// clone_fastが雛形。近くにあるアイテムを拾うようにする。

namespace {
  
typedef int Weight;
struct Edge {
  int src, dst;
  Weight weight;
  Edge(int src, int dst, Weight weight) :
    src(src), dst(dst), weight(weight) { }
};
bool operator < (const Edge& e, const Edge& f) {
  return e.weight != f.weight ? e.weight > f.weight : // !!INVERSE!!
  e.src != f.src ? e.src < f.src : e.dst < f.dst;
}
typedef vector<Edge> Edges;
typedef vector<Edges> Graph;

Graph graph;
Game* game;
std::vector<Point> route;

pair<Weight, Edges> minimumSpanningTree(const Graph& g, int r = 0) {
  int n = g.size();
  Edges T;
  Weight total = 0;

  vector<bool> visited(n);
  std::priority_queue<Edge> Q;
  Q.push(Edge(-1, r, 0));
  while (!Q.empty()) {
    Edge e = Q.top(); Q.pop();
    if (visited[e.dst]) continue;
    T.push_back(e);
    total += e.weight;
    visited[e.dst] = true;
    for (const auto& f : g[e.dst]) if (!visited[f.dst]) Q.push(f);
  }
  return pair<Weight, Edges>(total, T);
}

int toIndex(int x, int y) {
  return y * game->map2d.W + x;
}

int toIndex(const Point& p) {
  return toIndex(p.x, p.y);
}

Point toPoint(int index) {
  return Point(index % game->map2d.W, index / game->map2d.W);
}

struct WrapperEngine {
  WrapperEngine(Game *game, int id, int next_point_index) : m_game(game), m_id(id), m_wrapper(game->wrappers[id].get()), m_num_manipulators(0), m_next_point_index(next_point_index) { m_total_wrappers++; };
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
      while ((game->map2d(route[m_next_point_index]) & CellType::kWrappedBit) != 0) {
        m_next_point_index = (m_next_point_index + 1) % route.size();
      }

      auto pos = m_wrapper->pos;
      auto dst = route[m_next_point_index];
      std::vector<Trajectory> trajs = map_parse::findTrajectory(*game, pos, dst, DISTANCE_INF, false, false);

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
  int m_next_point_index;
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

std::string mstSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  ::game = game;

  std::random_device seed_gen;
  std::mt19937 engine(seed_gen());

  int W = game->map2d.W;
  int H = game->map2d.H;
  constexpr int DX[] = { 0, -1, 0, 1};
  constexpr int DY[] = { -1, 0, 1, 0};
  graph.resize(W * H);
  for (int x = 0; x < W; ++x) {
    for (int y = 0; y < H; ++y) {
      if ((game->map2d(x, y) & CellType::kObstacleBit) != 0) {
        continue;
      }

      for (int dir = 0; dir < 4; ++dir) {
        int xx = x + DX[dir];
        int yy = y + DY[dir];
        
        if (xx < 0 || W <= xx || yy < 0 || H <= yy) {
          continue;
        }

        if ((game->map2d(xx, yy) & CellType::kObstacleBit) != 0) {
          continue;
        }

        graph[toIndex(x, y)].emplace_back(toIndex(x, y), toIndex(xx, yy), 1);
      }
    }
  }

  auto mstResult = minimumSpanningTree(graph, toIndex(game->wrappers[0]->pos));

  Graph mstGraph(W * H);
  for (const auto& edge : mstResult.second) {
    if (edge.src == -1) {
      continue;
    }
    mstGraph[edge.src].push_back(edge);
  }

  std::vector<int> stk;
  std::vector<int> visited(W * H);
  stk.push_back(toIndex(game->wrappers[0]->pos));
  visited[toIndex(game->wrappers[0]->pos)] = true;
  while (!stk.empty()) {
    int from = stk.back();
    stk.pop_back();
    route.push_back(toPoint(from));

    for (const auto& edge : mstGraph[from]) {
      if (visited[edge.dst]) {
        continue;
      }

      visited[edge.dst] = true;
      stk.push_back(edge.dst);
    }
  }

  int num_wrappers = 1;
  vector<WrapperEngine> ws;
  bool clone_mode = false;
  ws.emplace_back(WrapperEngine(game, 0, 0));
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
      std::vector<int> unwrapped_point_indexes;
      for (int point_index = 0; point_index < route.size(); ++point_index) {
        if ((game->map2d(route[point_index]) & CellType::kWrappedBit) != 0) {
          unwrapped_point_indexes.push_back(point_index);
        }
      }

      int point_index;
      if (!unwrapped_point_indexes.empty()) {
        std::uniform_int_distribution<> distribution(0, unwrapped_point_indexes.size() - 1);
        point_index = distribution(engine);
      }
      else {
        point_index = 0;
      }

      ws.emplace_back(game, id, point_index);
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("mst", mstSolver);

#pragma optimize( "", on )
