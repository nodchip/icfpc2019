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

using TrajectoryMap = std::vector<std::vector<Trajectory>>;
using UpdateCallback = std::function<bool(Trajectory&, Trajectory&)>;

TrajectoryMap generateTrajectoryMap(const Game &game,
                                    const Point &from,
                                    const int max_dist,
                                    const UpdateCallback& update_callback) {
  const int kXMax = game.map2d.W;
  const int kYMax = game.map2d.H;

  TrajectoryMap traj_map(kYMax, std::vector<Trajectory>(kXMax));
  std::priority_queue<Trajectory> que;
  traj_map[from.y][from.x] =
    Trajectory{from, from, 0};

  que.push(traj_map[from.y][from.x]);
  while (!que.empty()) {
    Trajectory traj = que.top();
    que.pop();
    std::cout<<"spawn "<<traj<<std::endl;
    if (traj.distance > max_dist) {
      continue;
    }

    auto try_expand = [&](Direction dir) {
      int x_try = traj.to.x;
      int y_try = traj.to.y;
      switch (dir) {
      case Direction::W: ++y_try; break;
      case Direction::S: --y_try; break;
      case Direction::D: ++x_try; break;
      case Direction::A: --x_try; break;
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
      std::cout<<"try "<<traj_try<<std::endl;
      if (update_callback(traj_try, traj_map[y_try][x_try])) {
        que.push(traj_try);
	std::cout<<"ok"<<std::endl;
      }
    };

    try_expand(Direction::W);
    try_expand(Direction::A);
    try_expand(Direction::S);
    try_expand(Direction::D);
  }

  return traj_map;
}

Trajectory findTrajectory(const Game &game, const Point &from, const Point &to,
                          const int max_dist) {
  TrajectoryMap traj_map = generateTrajectoryMap(
      game, from, max_dist,
      [](Trajectory& traj_new, Trajectory& traj_orig) {
        if (traj_new < traj_orig) {
          traj_orig = traj_new;
          return true;  // Will enqueue |traj_new|
        }
        return false;  // Won't enqueue |traj_new|
      });

  return traj_map[to.y][to.x];
}

Trajectory findNearestUnwrapped(const Game &game, const Point& from, const int max_dist) {
  int nearest = DISTANCE_INF;
  Point nearest_point = {-1, -1};

  TrajectoryMap traj_map = generateTrajectoryMap(
      game, from, max_dist,
      [&](Trajectory& traj_new, Trajectory& traj_orig) {
        if (traj_new < traj_orig) {
          traj_orig = traj_new;
          if (game.map2d(traj_new.to.x, traj_new.to.y) == CellType::kEmpty &&
              traj_new.distance < nearest) {
            nearest_point = traj_new.to;
            nearest = traj_new.distance;
          } else {
            return true;  // Will enqueue |traj_new|
          }
        }
        return false;  // Won't enqueue |traj_new|
      });

  if (nearest == DISTANCE_INF){
    return Trajectory{from, from, 0}; // nothing to do
  }
  return traj_map[nearest_point.y][nearest_point.x];
}

} // namespace map_parse
