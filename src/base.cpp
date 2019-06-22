#include "base.h"

#include <algorithm>

Direction turn(Direction dir, bool turn_cw) {
  switch (dir) {
    case Direction::W: return turn_cw ? Direction::D : Direction::A;
    case Direction::A: return turn_cw ? Direction::W : Direction::S;
    case Direction::S: return turn_cw ? Direction::A : Direction::D;
    case Direction::D: return turn_cw ? Direction::S : Direction::W;
  }
  return Direction::UNKNOWN;
}

std::ostream& operator<<(std::ostream& os, const Point& p) {
    os << "(" << p.x << "," << p.y << ")";
    return os;
}

BoundingBox calcBoundingBox(const std::vector<Point>& points) {
  BoundingBox bbox;
  for (auto p : points) {
    bbox.lower.x = std::min(bbox.lower.x, p.x);
    bbox.lower.y = std::min(bbox.lower.y, p.y);
    bbox.upper.x = std::max(bbox.upper.x, p.x);
    bbox.upper.y = std::max(bbox.upper.y, p.y);
  }
  return bbox;
}
