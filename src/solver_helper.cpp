#include "solver_helper.h"
#include <iostream>
#include <cassert>
#include <queue>
#include "solver_utils.h"

std::string wrapperEngineSolver(SolverParam param, Game* game, SolverIterCallback iter_callback, WrapperEngineBase::Ptr prototype) {
  std::vector<WrapperEngineBase::Ptr> engines;
  engines.emplace_back(prototype->create(game, game->wrappers[0].get()));

  while (!game->isEnd()) {
    std::vector<int> cloned_ids;
    for (auto& e : engines) {
      std::vector<WrapperEngineBase*> other_engines;
      for (auto& o : engines) {
        if (e.get() != o.get()) {
            other_engines.push_back(o.get());
        }
      }
      if (Wrapper* e_cloned = e->action(other_engines)) {
        cloned_ids.emplace_back(e_cloned->index);
      }
    }

    game->tick();
    displayAndWait(param, game);
    if (iter_callback && !iter_callback(game)) return game->getCommand();

    for (auto id : cloned_ids) {
      engines.emplace_back(prototype->create(game, game->wrappers[id].get()));
    }
  }
  return game->getCommand();
}

std::string functorSolver(SolverParam param, Game* game, SolverIterCallback iter_callback, std::function<Wrapper*(Wrapper*)> func) {
  while (!game->isEnd()) {
    for (auto& w : game->wrappers) {
      func(w.get());
    }
    game->tick();
    displayAndWait(param, game);
    if (iter_callback && !iter_callback(game)) return game->getCommand();
  }
  return game->getCommand();

}
void ManipulatorExtender::extend() {
  assert (game->num_boosters[BoosterType::MANIPULATOR] > 0);
  if (num_attached_manipulators % 2 == 0) {
    wrapper->addManipulator(Point(1, 2 + num_attached_manipulators / 2));
  } else {
    wrapper->addManipulator(Point(1, - 2 - num_attached_manipulators / 2));
  }
  num_attached_manipulators++;
}

std::vector<std::vector<Point>> disjointConnectedComponentsByMask(const Map2D& map, int mask, int bits) {
  constexpr int BACKGROUND = 0;
  constexpr int FOREGROUND = 1;
  constexpr int VISITED = 2;
  Map2D work(map.W, map.H, BACKGROUND);
  for (int y = 0; y < map.H; ++y) {
    for (int x = 0; x < map.W; ++x) {
      if ((map(x, y) & mask) == bits) {
        work(x, y) = FOREGROUND;
      }
    }
  }

  std::vector<std::vector<Point>> components;
  for (int y = 0; y < work.H; ++y) {
    for (int x = 0; x < work.W; ++x) {
      if (work(x, y) == FOREGROUND) {
        // start BFS fill
        std::vector<Point> component;
        std::queue<Point> que;
        que.push({x, y});
        work(x, y) = VISITED;
        component.push_back({x, y});
        while (!que.empty()) {
          Point p = que.front(); que.pop();
          for (auto offset : all_directions) {
            Point n = p + Point(offset);
            if (work.isInside(n) && work(n) == FOREGROUND) {
              que.push(n);
              work(n) = VISITED;
              component.push_back(n);
            }
          }
        }
        components.push_back(component);
      }
    }
  }
  return components;
}

std::unique_ptr<FindFCRouteResult> findGoodFCRoute(const Map2D& map, Point start) {
  auto Fs = enumerateCellsByMask(map, CellType::kBoosterFastWheelBit, CellType::kBoosterFastWheelBit);
  auto Cs = enumerateCellsByMask(map, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit);
  std::cout << "Fs:" << Fs.size() << " Cs:" << Cs.size() << std::endl;
  if (Fs.empty() || Cs.empty()) return {};

  // start -> Fs[i] -> Cs[j]
  std::vector<FindFCRouteResult> res;
  // start -> Fs[i]
  for (int i = 0; i < Fs.size(); ++i) {
    auto path_to_F = shortestPathByMaskBFS(map, CellType::kObstacleBit, 0, start, {Fs[i]});
    std::cout << "path_to_F:" << path_to_F.size() << std::endl;
    for (auto p : path_to_F) std::cout << p;
    std::cout << std::endl;
    if (!path_to_F.empty() && path_to_F.front() == start && path_to_F.back() == Fs[i]) {
      // start -> Fs[i]
      for (int j = 0; j < Cs.size(); ++j) {
        auto path_to_C = shortestPathByMaskBFS(map, CellType::kObstacleBit, 0, Fs[i], {Cs[j]});
        std::cout << "path_to_C:" << path_to_C.size() << std::endl;
        for (auto p : path_to_C) std::cout << p;
        std::cout << std::endl;
        if (!path_to_C.empty() && path_to_C.front() == Fs[i] && path_to_C.back() == Cs[j]) {
          std::cout << "s-F:" << path_to_F.size() << " F-C:" << path_to_C.size() << std::endl;
          FindFCRouteResult candidate;
          candidate.F_pos = Fs[i];
          candidate.C_pos = Cs[j];
          candidate.time_cost += path_to_F.size() - 1; // start -> Fs[i]
          candidate.time_cost += 1; // use F
          // Fs[i] -> Cs[j]
          const int lenC = path_to_C.size() - 1;
          candidate.time_cost += std::max(lenC - 30, lenC / 2);
          res.push_back(candidate);
        }
      }
    }
  }
  // start -> Cs[j]
  const Point invalid {-1, -1};
  for (int j = 0; j < Cs.size(); ++j) {
    auto path_to_C = shortestPathByMaskBFS(map, CellType::kObstacleBit, 0, start, {Cs[j]});
    std::cout << "direct path_to_C:" << path_to_C.size() << std::endl;
    for (auto p : path_to_C) std::cout << p;
    std::cout << std::endl;
    if (!path_to_C.empty() && path_to_C.front() == start && path_to_C.back() == Fs[j]) {
      FindFCRouteResult candidate;
      candidate.F_pos = invalid;
      candidate.C_pos = Cs[j];
      candidate.time_cost += path_to_C.size() - 1; // start -> Cs[i]
      res.push_back(candidate);
    }
  }
  auto it_min = std::min_element(res.begin(), res.end(), [](auto lhs, auto rhs) { return lhs.time_cost < rhs.time_cost; });
  if (it_min == res.end()) return {};
  if (it_min->F_pos == invalid) return {};

  std::unique_ptr<FindFCRouteResult> result(new FindFCRouteResult);
  *result.get() = *it_min;
  return result;
}

namespace detail {
// http://www.prefield.com/algorithm/math/hungarian.html + mod.
using weight = int;
using matrix = std::vector<std::vector<weight>>;
weight hungarian(const matrix &a, std::vector<int>& x, std::vector<int>& y) {
  using namespace std;
  constexpr int INF = std::numeric_limits<int>::max();
  int n = a.size(), p, q;
  vector<int> fx(n, INF), fy(n, 0);
  x.assign(n, -1);
  y.assign(n, -1);
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      fx[i] = max(fx[i], a[i][j]);
  for (int i = 0; i < n; ) {
    vector<int> t(n, -1), s(n+1, i);
    for (p = q = 0; p <= q && x[i] < 0; ++p)
      for (int k = s[p], j = 0; j < n && x[i] < 0; ++j)
        if (fx[k] + fy[j] == a[k][j] && t[j] < 0) {
          s[++q] = y[j], t[j] = k;
          if (s[q] < 0)
            for (p = j; p >= 0; j = p)
              y[j] = k = t[j], p = x[k], x[k] = j;
        }
    if (x[i] < 0) {
      weight d = INF;
      for (int k = 0; k <= q; ++k)
        for (int j = 0; j < n; ++j)
          if (t[j] < 0) d = min(d, fx[s[k]] + fy[j] - a[s[k]][j]);
      for (int j = 0; j < n; ++j) fy[j] += (t[j] < 0 ? 0 : d);
      for (int k = 0; k <= q; ++k) fx[s[k]] -= d;
    } else ++i;
  }
  weight ret = 0;
  for (int i = 0; i < n; ++i) ret += a[i][x[i]];
  return ret;
}
} // detail

ConnectedComponentAssignmentForParanoid::ConnectedComponentAssignmentForParanoid(
  Game* game_, int distance_threshold_, int small_region_bonus_)
  : game(game_), distance_threshold(distance_threshold_), small_region_bonus(small_region_bonus_) {
}

bool ConnectedComponentAssignmentForParanoid::hasDisjointComponents() const {
  return components.size() > 1;
}
bool ConnectedComponentAssignmentForParanoid::isComponentAssignedToWrapper(int i) const {
  return !wrapper_to_component.empty() // no assign
    && 0 <= i && i < wrapper_to_component.size() // invalid
    && 0 <= wrapper_to_component[i] && wrapper_to_component[i] < components.size() // #wrapper > #cc
    && !components[wrapper_to_component[i]].points.empty() // non-empty component
    ;
}
Point ConnectedComponentAssignmentForParanoid::getTargetOfWrapper(int i) const {
  if (!isComponentAssignedToWrapper(i)) return Point {-1, -1};
  return components[wrapper_to_component[i]].points.front();
}

Point ConnectedComponentAssignmentForParanoid::getSuggestedMotionOfWrapper(int i) const {
  if (!isComponentAssignedToWrapper(i)) return Point {0, 0};
  return components[wrapper_to_component[i]].suggested_motion;
}

void ConnectedComponentAssignmentForParanoid::delayUpdate() {
  delay_update_flag = true;
}

bool ConnectedComponentAssignmentForParanoid::update() {
  if (!delay_update_flag) return false;
  delay_update_flag = false;

  components.clear();
  wrapper_to_component.clear();
  auto ccs = disjointConnectedComponentsByMask(game->map2d, 0b11, 0b00);
  if (ccs.empty()) {
    return true;
  }

  // sort by its size. it is used later for small-region bonus.
  std::sort(ccs.begin(), ccs.end(), [](auto& lhs, auto& rhs) { return lhs.size() < rhs.size(); });

  for (auto cc : ccs) {
    components.push_back({ cc, approximateCenter(cc) });
  }

  const int N = game->wrappers.size();
  const int M = ccs.size();
  std::vector<int> component_to_wrapper;

  // hungarian method
  const int sz = std::max(N, M);
  detail::matrix preference(sz, std::vector<int>(sz, 0)); // preference[wrapper][component]
  // to avoid occilation, it is suggested to use the motion found in the shortestPathByMaskBFS.
  std::vector<std::vector<Point>> suggested_motion(sz, std::vector<Point>(sz, {0, 0}));
  std::vector<std::vector<Point>> target(sz, std::vector<Point>(sz, {-1, -1}));
  for (int i = 0; i < sz; ++i) {
    for (int j = 0; j < sz; ++j) {
      if (i < N && j < M) {
        Point pos = game->wrappers[i]->pos;
        // 中心までの距離を見ることで、自然に小さいものを好むようにしつつ、遠くのwrapperの作業に気を取られないようにする
#if 0
        // 本来は中心までの距離ではなく、CCまでの最短経路長を見るべき
        const int distance2 = (components[j].center - pos).length2();
        if (distance2 < distance_threshold) {
          preference[i][j] = distance_threshold - distance2;
        }
#else
        // 簡易的に領域重心までの直線距離を用いていたが、最短経路と反する場合に振動する
        // 仕方ないので、正しく最短経路を計算する(重い)
        // 今まさに小領域と大領域に分割してしまった場合、当初の小面積基準や重心距離基準では正しく選択できたが、
        // 領域のどこかへの最短経路の場合はどちらも同程度(~ 1)となり、小領域を優先して潰すことにならないことがある。
        //
        // ターゲットは重心位置に最も近い領域内の点としつつ、距離は経路で測定するのが良さそうと思ったが、
        // 何故か今の所うまく動いておらず逆に小領域が残る (1)
        int nearest_manhattan = game->map2d.W + game->map2d.H;
        for (auto p : components[j].points) {
          nearest_manhattan = std::min(nearest_manhattan, (pos - p).lengthManhattan());
        }
        if (nearest_manhattan < distance_threshold) {
          Map2D map(game->map2d.W, game->map2d.H, 0);
          std::vector<Point> path = shortestPathByMaskBFS(game->map2d,
            CellType::kObstacleBit, 0, // inside room
            //pos, findNearestPoints(components[j].points, components[j].center)); // (1)
            pos, components[j].points);
          if (path.size() < distance_threshold) {
            preference[i][j] = distance_threshold - path.size();
            if (true) {
              // add small-region bonus.
              // components are sorted. (j==0: smallest, j==M-1: largest.)
              preference[i][j] += small_region_bonus * (M - j); // large preference for small j.
            }
            if (path.size() >= 2) {
              suggested_motion[i][j] = path[1] - path[0];
              target[i][j] = path.back();
            }
          }
        }
#endif
      }
    }
  }

  detail::hungarian(preference, wrapper_to_component, component_to_wrapper);
  for (int i = 0; i < sz; ++i) {
    const int j = wrapper_to_component[i];
    if (j != UNASSIGNED && j < M) {
      components[j].suggested_motion = suggested_motion[i][j];
      components[j].target = target[i][j];
    }
  }

  return true;
}

Point ConnectedComponentAssignmentForParanoid::approximateCenter(const std::vector<Point>& points) {
  double x = 0;
  double y = 0;
  for (auto p : points) x += p.x, y += p.y;
  return {int(x / points.size()), int(y / points.size())};
}

std::vector<Point> ConnectedComponentAssignmentForParanoid::findNearestPoints(const std::vector<Point>& haystack, Point needle) {
  int best_dist = std::numeric_limits<int>::max();
  std::vector<Point> best_points;
  for (auto h : haystack) {
    const int dist = (h - needle).lengthManhattan();
    if (dist == best_dist) {
      best_points.push_back(h);
    } else if (dist < best_dist) {
      best_dist = dist;
      best_points = {h};
    }
  }
  return best_points;
}