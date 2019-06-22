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
  std::queue<Trajectory> que;
  traj_map[from.y][from.x] =
    Trajectory{Direction::W, from, 0, false};

  que.push(traj_map[from.y][from.x]);
  while (!que.empty()) {
    Trajectory traj = que.front();
    que.pop();
    // std::cout<<"spawn "<<traj<<std::endl;
    if (traj.distance > max_dist) {
      continue;
    }

    auto try_expand = [&](Direction dir) {
      int x_try = traj.pos.x;
      int y_try = traj.pos.y;
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
      traj_try.distance += 1;
      traj_try.last_move = dir;
      traj_try.pos = {x_try, y_try};
      // std::cout<<"try "<<traj_try<<std::endl;
      if (update_callback(traj_try, traj_map[y_try][x_try])) {
        que.push(traj_try);
	// std::cout<<"ok"<<std::endl;
      }
    };

    try_expand(Direction::W);
    try_expand(Direction::A);
    try_expand(Direction::S);
    try_expand(Direction::D);
  }

  return traj_map;
}

std::vector<Trajectory> findTrajectory(const Game &game, const Point &from, const Point &to,
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


  const int dist_out = traj_map[to.y][to.x].distance;
  std::vector<Trajectory> trajs(dist_out);

  Point pos(to.x, to.y);
  for(int i=0; i<dist_out; ++i){
    trajs[dist_out - i - 1] = traj_map[pos.y][pos.x];
    switch (traj_map[pos.y][pos.x].last_move) {
    case Direction::W: --pos.y; break;
    case Direction::S: ++pos.y; break;
    case Direction::D: --pos.x; break;
    case Direction::A: ++pos.x; break;
    }
  }
  return trajs;  
}

std::vector<Trajectory> findNearestUnwrapped(const Game &game, const Point& from, const int max_dist) {
  static constexpr int kMask = CellType::kObstacleBit | CellType::kWrappedBit;
  int nearest = DISTANCE_INF;
  Point nearest_point = {-1, -1};

  TrajectoryMap traj_map = generateTrajectoryMap(
      game, from, max_dist,
      [&](Trajectory& traj_new, Trajectory& traj_orig) {
        if (traj_new < traj_orig) {
          traj_orig = traj_new;
          if ((game.map2d(traj_new.pos.x, traj_new.pos.y) & kMask) == 0 &&
              traj_new.distance < nearest) {
            nearest_point = traj_new.pos;
            nearest = traj_new.distance;
          } else if(traj_new.distance < nearest){
	    return true;  // Will enqueue |traj_new|
          }
        }
        return false;  // Won't enqueue |traj_new|
      });

  if (nearest == DISTANCE_INF){
    return std::vector<Trajectory>(0);
  }

  const int dist_out = traj_map[nearest_point.y][nearest_point.x].distance;
  std::vector<Trajectory> trajs(dist_out);

  Point pos(nearest_point.x, nearest_point.y);
  for(int i=0; i<dist_out; ++i){
    trajs[dist_out - i - 1] = traj_map[pos.y][pos.x];
    switch (traj_map[pos.y][pos.x].last_move) {
    case Direction::W: --pos.y; break;
    case Direction::S: ++pos.y; break;
    case Direction::D: --pos.x; break;
    case Direction::A: ++pos.x; break;
    }
  }
  return trajs;

}

  std::vector<Trajectory> findNearestByBit(const Game &game, const Point& from, const int max_dist, const int kMask) {

  int nearest = DISTANCE_INF;
  Point nearest_point = {-1, -1};

  TrajectoryMap traj_map = generateTrajectoryMap(
      game, from, max_dist,
      [&](Trajectory& traj_new, Trajectory& traj_orig) {
        if (traj_new < traj_orig) {
          traj_orig = traj_new;
          if ((game.map2d(traj_new.pos.x, traj_new.pos.y) & kMask) != 0 &&
              traj_new.distance < nearest) {
            nearest_point = traj_new.pos;
            nearest = traj_new.distance;
          } else if(traj_new.distance < nearest){
	    return true;  // Will enqueue |traj_new|
          }
        }
        return false;  // Won't enqueue |traj_new|
      });

  if (nearest == DISTANCE_INF){
    return std::vector<Trajectory>(0);
  }

  const int dist_out = traj_map[nearest_point.y][nearest_point.x].distance;
  std::vector<Trajectory> trajs(dist_out);

  Point pos(nearest_point.x, nearest_point.y);
  for(int i=0; i<dist_out; ++i){
    trajs[dist_out - i - 1] = traj_map[pos.y][pos.x];
    switch (traj_map[pos.y][pos.x].last_move) {
    case Direction::W: --pos.y; break;
    case Direction::S: ++pos.y; break;
    case Direction::D: --pos.x; break;
    case Direction::A: ++pos.x; break;
    }
  }
  return trajs;

}
  
} // namespace map_parse
