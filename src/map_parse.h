#pragma once

#include <vector>

#include "trajectory.h"
#include "map.h"

const int MAP_XMAX = 256;
const int MAP_YMAX = 256;

struct traj_evaluator{
  bool comp_traj(const Trajectory &t1, const Trajectory &t2);
};
namespace map_parse{
  Trajectory find_trajectory(const Map &map, const Point from, const Point to, const int max_dist);
  Trajectory find_nearest_unwrapped(const Map &map, const Point from, const int max_dist);
  void test_map_parse();
}
