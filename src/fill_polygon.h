#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "base.h"

namespace detail {
struct VerticalLine {
    int x = 0;
    int y0 = 0;
    int y1 = 0;
};
std::vector<VerticalLine> EnumerateIntersectionsToHorizontalLine(int line_y, const Polygon& polygon);
}

// set map[y][x] := value inside the polygon = [(x0, y1), ..]
bool FillPolygon(Map2D& map, const Polygon& polygon, int value);