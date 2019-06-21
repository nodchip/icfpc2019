#include "base.h"
#include <iostream>

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

std::ostream& operator<<(std::ostream& os, const Map2D& map) {
    for (int y = 0; y < map.H; ++y) {
        for (int x = 0; x < map.W; ++x) {
            std::cout << map(x, y);
        }
        std::cout << std::endl;
    }
    return os;
}