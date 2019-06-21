#pragma once
#include <vector>
#include "base.h"
#include "game_map.h"

std::vector<Point> requiredClearance(Point offset);

// manipulator_offsets: relative to wrappy_pos
// result: absolute position.
std::vector<Point> absolutePositionOfReachableManipulators(
  const Map2D& map2d, Point wrappy_pos, const std::vector<Point>& relative_manipulator_offsets);