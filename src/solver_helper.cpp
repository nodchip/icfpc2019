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
    auto path_to_F = nearestPathByMaskBFS(map, CellType::kObstacleBit, 0, start, {Fs[i]});
    std::cout << "path_to_F:" << path_to_F.size() << std::endl;
    for (auto p : path_to_F) std::cout << p; std::cout << std::endl;
    if (!path_to_F.empty() && path_to_F.front() == start && path_to_F.back() == Fs[i]) {
      // start -> Fs[i]
      for (int j = 0; j < Cs.size(); ++j) {
        auto path_to_C = nearestPathByMaskBFS(map, CellType::kObstacleBit, 0, Fs[i], {Cs[j]});
        std::cout << "path_to_C:" << path_to_C.size() << std::endl;
        for (auto p : path_to_C) std::cout << p; std::cout << std::endl;
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
    auto path_to_C = nearestPathByMaskBFS(map, CellType::kObstacleBit, 0, start, {Cs[j]});
    std::cout << "direct path_to_C:" << path_to_C.size() << std::endl;
    for (auto p : path_to_C) std::cout << p; std::cout << std::endl;
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