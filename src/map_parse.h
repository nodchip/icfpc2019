#pragma once

#include <vector>

#include "game.h"
#include "trajectory.h"

namespace map_parse {

  std::vector<Trajectory> findTrajectory(const Game &game, const Point &from, const Point &to,
                          const int max_dist);
  std::vector<Trajectory> findNearestUnwrapped(const Game &game, const Point &from,
                                const int max_dist);

} // namepsace map_parse
