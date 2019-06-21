#include "map_parse.h"

#include <functional>
#include <iostream>
#include <queue>

namespace std {

// 経路の価値の比較。評価関数に相当する
template <> struct less<Trajectory> {
  bool operator()(const Trajectory &t1, const Trajectory &t2) {
    // とりあえず距離が短いほうが偉いとする
    return t1.distance < t2.distance;
  }
};

} // namespace std

namespace map_parse {

bool operator<(const Trajectory &t1, const Trajectory &t2) {
  return std::less<Trajectory>()(t1, t2);
}

Trajectory findTrajectory(const Game &game, const Point &from, const Point &to,
                          const int max_dist) {
  const int kXMax = game.map2d.W;
  const int kYMax = game.map2d.H;

  std::vector<std::vector<Trajectory>> traj_map(kYMax,
                                                std::vector<Trajectory>(kXMax));
  std::priority_queue<Trajectory> que;
  traj_map[from.y][from.x] =
      Trajectory{from, from, 0, false, std::vector<Direction>(0)};

  que.push(traj_map[from.y][from.x]);
  while (!que.empty()) {
    Trajectory traj = que.top();
    que.pop();

    if (traj.distance > max_dist) {
      continue;
    }

    auto try_expand = [&](Direction dir) {
      int x_try = traj.to.x;
      int y_try = traj.to.y;
      switch (dir) {
      case W: ++y_try; break;
      case S: --y_try; break;
      case D: ++x_try; break;
      case A: --x_try; break;
      }

      if (x_try > kXMax - 1 || x_try < 0 || y_try > kYMax - 1 || y_try < 0) {
        return;
      }
      if (game.map2d(x_try, y_try) & CellType::kObstacleBit) {
        // todo write drill
        return;
      }

      Trajectory traj_try = traj;
      traj_try.path.push_back(dir);
      traj_try.distance += 1;
      traj_try.from = traj_try.to;
      traj_try.to = {x_try, y_try};
      if (traj_try < traj_map[y_try][x_try]) {
        traj_map[y_try][x_try] = traj_try;
        que.push(traj_try);
      }
    };

    try_expand(Direction(W));
    try_expand(Direction(A));
    try_expand(Direction(S));
    try_expand(Direction(D));
  }

  return traj_map[to.y][to.x];
}

Trajectory findNearestUnwrapped(const Game &game, const Point& from, const int max_dist) {
  const int kXMax = game.map2d.W;
  const int kYMax = game.map2d.H;

  std::vector<std::vector<Trajectory>> traj_map(kYMax, std::vector<Trajectory>(kXMax));
  std::priority_queue<Trajectory> que;
  traj_map[from.y][from.x] = Trajectory { from, from, 0, false, std::vector<Direction>(0) };
  que.push(traj_map[from.y][from.x]);

  int nearest = DISTANCE_INF;
  Point nearest_point = {-1, -1};

  while (!que.empty()) {
    Trajectory traj = que.top();
    que.pop();

    if (traj.distance > max_dist || traj.distance > nearest) {
      continue;
    }

    auto try_expand = [&](Direction dir) {
      int x_try = traj.to.x;
      int y_try = traj.to.y;
      switch (dir) {
      case W: ++y_try; break;
      case S: --y_try; break;
      case D: ++x_try; break;
      case A: --x_try; break;
      }

      if (x_try > kXMax - 1 || x_try < 0 || y_try > kYMax - 1 || y_try < 0) {
        return;
      }
      if (game.map2d(x_try, y_try) & CellType::kObstacleBit) {
        // TODO: write drill
        return;
      }

      Trajectory traj_try = traj;
      traj_try.path.push_back(dir);
      traj_try.distance += 1;
      traj_try.from = traj_try.to;
      traj_try.to = {x_try, y_try};
      if (traj_try < traj_map[y_try][x_try]) {
        traj_map[y_try][x_try] = traj_try;
        if (game.map2d(x_try, y_try) == CellType::kEmpty &&
            traj_try.distance < nearest) {
          nearest_point = {x_try, y_try};
          nearest = traj_try.distance;
        } else {
          que.push(traj_try);
        }
      }
    };

    try_expand(Direction(W));
    try_expand(Direction(A));
    try_expand(Direction(S));
    try_expand(Direction(D));
  }

  return traj_map[nearest_point.y][nearest_point.x];
}

} // namespace map_parse
