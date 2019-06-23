#pragma once

#include <vector>

#include "game.h"
#include "trajectory.h"

namespace map_parse {

  std::vector<Trajectory> findTrajectory(const Game &game, const Point &from, const Point &to,
					 const int max_dist, const bool dstart=false, const bool astart=false);
  std::vector<Trajectory> findNearestUnwrapped(const Game &game, const Point &from,
					       const int max_dist, const bool dstart=false, const bool astart=false);

  std::vector<Trajectory> findNearestByBit(const Game &game, const Point &from,
					   const int max_dist, const int kMask, const bool dstart=false, const bool astart=false);
  

} // namepsace map_parse
