#include "base.h"

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
