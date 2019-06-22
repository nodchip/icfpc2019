#include "fill_polygon.h"

#include <algorithm>
#include <cassert>

namespace detail {
// O(N)
std::vector<VerticalLine> enumerateIntersectionsToHorizontalLine(int line_y, const Polygon& polygon) {
    std::vector<VerticalLine> res;
    for (size_t i = 0; i < polygon.size(); ++i) {
        const size_t j = (i + polygon.size() + 1) % polygon.size();
        if (polygon[i].x == polygon[j].x) {
            assert(polygon[i].y != polygon[j].y); // rectilinear lines
            VerticalLine line;
            line.x = polygon[i].x;
            line.y0 = std::min(polygon[i].y, polygon[j].y);
            line.y1 = std::max(polygon[i].y, polygon[j].y);
            if (line.y0 <= line_y && line_y < line.y1) {
                res.push_back(line);
            }
        } else {
            assert(polygon[i].y == polygon[j].y); // rectilinear lines
        }
    }
    return res;
}
}

// scan-line method
bool fillPolygon(Map2D& map, const Polygon& polygon, int value) {
    using namespace detail;
    for (int y = 0; y < map.H; ++y) {
        auto intersections = enumerateIntersectionsToHorizontalLine(y, polygon);
        std::sort(intersections.begin(), intersections.end(), [](const VerticalLine& lhs, const VerticalLine& rhs) {
            return lhs.x < rhs.x;
        });
        bool inside = false;
        int last_x = -1;
        for (auto line : intersections) {
            if (inside) {
                // paint [last, current)
                for (int x = last_x; x < line.x; ++x) {
                    map(x, y) = value;
                }
            } else {
                last_x = line.x;
            }
            inside = !inside;

        }
    }

    return true;
}