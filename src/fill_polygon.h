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
bool pointsOnAxisAlignedLine(Point p0, Point p1, Point p2);
}

// set map[y][x] := value inside the polygon = [(x0, y1), ..]
bool fillPolygon(Map2D& map, const Polygon& polygon, int value);

// create a 4-connected left-inside polygon of inside: map[y][x] == value.
bool parsePolygon(Polygon& polygon, const Map2D& map, int value);

// create a sparse, simplified polygon 
// assumes that the first point is a corner (not a midpoint) which is returned by parsePolygon()
Polygon simplifyPolygon(Polygon& polygon);