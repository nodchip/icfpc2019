#pragma once

#include <vector>

#include "trajectory.h"
#include "game.h"

namespace map_parse{
  std::vector<Trajectory> find_trajectory(const Game &game, const Point from, const Point to);
  std::vector<Trajectory> find_nearest_unwrapped(const Game &game, const Point from, const int max_dist);
  
}
