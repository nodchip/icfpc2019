#pragma once

#include <ostream>
#include <vector>
#include <limits>

enum class Direction : std::uint8_t { W, S, A, D, UNKNOWN };
Direction turn(Direction dir, bool turn_cw);
inline Direction turnCCW(Direction dir) { return turn(dir, false); }
inline Direction turnCW(Direction dir) { return turn(dir, true); }

struct Point {
    int x = 0;
    int y = 0;
    Point() = default;
    Point(int x_, int y_) : x(x_), y(y_) {}
    bool operator==(const Point& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const Point& rhs) const { return !operator==(rhs); }
    Point operator+(const Point& rhs) const { return Point(x + rhs.x, y + rhs.y); }
    Point operator-(const Point& rhs) const { return Point(x - rhs.x, y - rhs.y); }
    int length2() const { return x * x + y * y; }
};
std::ostream& operator<<(std::ostream&, const Point&);

using Polygon = std::vector<Point>;

struct BoundingBox {
    Point lower { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    Point upper { std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };
    bool isValid() const {
        return lower.x < upper.x && lower.y < upper.y;
    }
};
BoundingBox calcBoundingBox(const std::vector<Point>& points);
