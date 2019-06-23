#include "solver_helper.h"
#include <cassert>
#include <queue>

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