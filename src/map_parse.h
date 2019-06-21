#pragma once

#include <vector>

#include "trajectory.h"
#include "game.h"

const int MAP_XMAX = 256;
const int MAP_YMAX = 256;

struct traj_evaluator{
  static bool comp_traj(const Trajectory &t1, const Trajectory &t2);
};
namespace map_parse{
  Trajectory find_trajectory(const Game &game, const Point from, const Point to, const int max_dist);
  Trajectory find_nearest_unwrapped(const Game &game, const Point from, const int max_dist);
  void test_map_parse();

}
