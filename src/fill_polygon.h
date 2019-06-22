#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "base.h"
#include "map2d.h"

namespace detail {
struct VerticalLine {
    int x = 0;
    int y0 = 0;
    int y1 = 0;
};
std::vector<VerticalLine> enumerateIntersectionsToHorizontalLine(int line_y, const Polygon& polygon);
}

// set map[y][x] := value inside the polygon = [(x0, y1), ..]
bool fillPolygon(Map2D& map, const Polygon& polygon, int value);

// create a 4-connected left-inside polygon of inside: map[y][x] == value.
bool parsePolygon(Polygon& polygon, const Map2D& map, int value);