#include "manipulator_reach.h"
#include <iostream>

std::vector<Point> RequiredClearance(Point offset) {
  std::vector<Point> res;
  int dx = std::abs(offset.x);
  int dy = std::abs(offset.y);
  int sx = 0 < offset.x ? 1 : -1;
  int sy = 0 < offset.y ? 1 : -1;
  int x = 0;
  int y = 0;
  int err = 0;
  while (true) {
    res.push_back({x, y});
    //std::cout << "pos = " << x << "," << y << " err=" << err << std::endl;
    if (Point(x, y) == offset) break;
    int err1 = err + dy;
    int err2 = err - dx;
    if (std::abs(err1) == std::abs(err2)) {
      x += sx;
      y += sy;
      err = err + dy - dx;
    } else if (std::abs(err1) < std::abs(err2)) {
      err = err1;
      x += sx;
    } else {
      err = err2;
      y += sy;
    }
  }
  return res;
}

std::vector<Point> AbsolutePositionOfReachableManipulators(
  const Map2D& map2d, Point wrappy_pos, const std::vector<Point>& relative_manipulator_offsets) {

  std::vector<Point> reachables;
  for (auto& manipulator : relative_manipulator_offsets) {
    bool blocked = false;
    for (auto& p : RequiredClearance(manipulator)) {
      auto test_pos = wrappy_pos + p;
      if (!map2d.isInside(test_pos) || (map2d(test_pos) & CellType::kObstacleBit) != 0) {
        //std::cout << "Clearance " << p << " test " << test_pos << " blocked" << std::endl;
        blocked = true;
        break;
      } else {
        //std::cout << "Clearance " << p << " test " << test_pos << " OK" << std::endl;
      }
    }
    if (!blocked) {
      reachables.push_back(wrappy_pos + manipulator);
    }
  }
  
  return reachables;
}