#pragma once

#include <ostream>
#include <vector>
#include <limits>

enum class Direction : std::uint8_t { W, S, A, D };
extern const std::vector<Direction> all_directions;
Direction turn(Direction dir, bool turn_cw);
inline Direction turnCCW(Direction dir) { return turn(dir, false); }
inline Direction turnCW(Direction dir) { return turn(dir, true); }

struct Point {
    int x = 0;
    int y = 0;
    Point() = default;
    explicit Point(Direction dir) {
        switch (dir) {
            case Direction::W: y = +1; break;
            case Direction::A: x = -1; break;
            case Direction::S: y = -1; break;
            case Direction::D: x = +1; break;
        }
    }
    Point(int x_, int y_) : x(x_), y(y_) {}
    bool operator==(const Point& rhs) const { return x == rhs.x && y == rhs.y; }
    bool operator!=(const Point& rhs) const { return !operator==(rhs); }
    Point operator+(const Point& rhs) const { return Point(x + rhs.x, y + rhs.y); }
    Point operator-(const Point& rhs) const { return Point(x - rhs.x, y - rhs.y); }
    int length2() const { return x * x + y * y; }
    int lengthManhattan() const { return std::abs(x) + std::abs(y); }
};
extern const std::vector<Point> neighbors4;
extern const std::vector<Point> neighbors8;

inline bool pointToDirection(Direction& d, const Point& p) {
    if (p == Point { 0,  1}) { d = Direction::W; return true; }
    if (p == Point {-1,  0}) { d = Direction::A; return true; }
    if (p == Point { 0, -1}) { d = Direction::S; return true; }
    if (p == Point { 1,  0}) { d = Direction::D; return true; }
    return false;
}

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
