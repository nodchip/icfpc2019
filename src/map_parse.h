#pragma once

#include <vector>

#include "trajectory.h"
#include "map.h"

namespace map_parse{
  std::vector<Trajectory> find_trajectory(const Map &map, const Point from, const Point to);
  std::vector<Trajectory> find_nearest_unwrapped(const Map &map, const Point from, const int max_dist);
  
}
