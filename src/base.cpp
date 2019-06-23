#include "base.h"

#include <algorithm>

const std::vector<Direction> all_directions = {
  Direction::W, Direction::A, Direction::S, Direction::D, 
};

const std::vector<Point> neighbors4 = {
  {0, 1}, {-1, 0}, {0, -1}, {1, 0}
};
const std::vector<Point> neighbors8 = {
  {-1, -1}, {-1, 0}, {-1, 1},
  { 0, -1}, { 0, 0}, { 0, 1},
  { 1, -1}, { 1, 0}, { 1, 1},
};


Direction turn(Direction dir, bool turn_cw) {
  switch (dir) {
    case Direction::W: return turn_cw ? Direction::D : Direction::A;
    case Direction::A: return turn_cw ? Direction::W : Direction::S;
    case Direction::S: return turn_cw ? Direction::A : Direction::D;
    case Direction::D: return turn_cw ? Direction::S : Direction::W;
    default: return Direction::D;
  }
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
