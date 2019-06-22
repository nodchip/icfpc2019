#include "fill_polygon.h"

#include <iostream>
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

bool parsePolygon(Polygon& polygon, const Map2D& map, int value) {
    // raster scan to find left & down free pixel
    //   CW
    //   +......+
    //   ^ ps   :
    //   |      :
    // --o.....>+ straight
    //   : pa   :
    //   v      :
    //   +......+
    //  CCW
    for (int y = 0; y < map.H; ++y) {
        for (int x = 0; x < map.W; ++x) {
            if (map(x, y) == value) {
                std::cout << "start point" << Point(x, y) << std::endl;
                // make a left-inside polygon (CCW tracing)
                Polygon fine = {{x, y}}; // always a corner and not a midpoint.
                Direction dir = Direction::D;
                do {
                    Point ps = fine.back(); // straight
                    Point pa = fine.back(); // angle
                    switch (dir) {
                        case Direction::W: ps.x -= 1; break;
                        case Direction::A: pa.x -= 1; ps.x -= 1; ps.y -= 1; break;
                        case Direction::S: pa.y -= 1; ps.y -= 1; pa.x -= 1; break;
                        case Direction::D: pa.y -= 1; break;
                    }
                    const bool ps_on = map.isInside(ps) && map(ps) == value;
                    const bool pa_on = map.isInside(pa) && map(pa) == value;
                    //std::cout << "point" << fine.back() << " dir is " << int(dir) << " ps:" << (ps_on ? 1 : 0) << " pa:" << (pa_on ? 1 : 0) << std::endl;
                    if (pa_on) {
                        assert (ps_on); // 4-connected
                        dir = turnCW(dir);
                    } else if (!ps_on) {
                        dir = turnCCW(dir);
                    }
                    fine.push_back(fine.back() + Point(dir));
                } while (fine.back() != Point {x, y});

                polygon = fine;
                return true;
            }
        }
    }
    return false; // no pixel found.
}