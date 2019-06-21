#pragma once

#include <ostream>
#include <vector>
#include <limits>

struct Point {
    int x = 0;
    int y = 0;
    Point() = default;
    Point(int x_, int y_) : x(x_), y(y_) {}
};

using Polygon = std::vector<Point>;

struct BoundingBox {
    Point lower { std::numeric_limits<int>::max(), std::numeric_limits<int>::max() };
    Point upper { std::numeric_limits<int>::min(), std::numeric_limits<int>::min() };
    bool isValid() const {
        return lower.x < upper.x && lower.y < upper.y;
    }
};
BoundingBox calcBoundingBox(const std::vector<Point>& points);

struct Map2D {
    using T = int;
    int W = 0;
    int H = 0;
    std::vector<T> data;

    Map2D() : Map2D(0, 0) {}
    Map2D(int W_, int H_, int value = 0) : W(std::max(W_, 0)), H(std::max(H_, 0)) {
        data.assign(W * H, value);
    }
    Map2D(int W_, int H_, std::initializer_list<int> vals) : Map2D(W_, H_) {
        //assert (vals.size() == W * H);
        auto it = vals.begin();
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x, ++it) {
                (*this)(x, y) = *it;
            }
        }
    }

    T& operator()(int x, int y) { return data[y * W + x]; }
    T operator()(int x, int y) const { return data[y * W + x]; }
    bool operator!=(const Map2D& rhs) const {
        return !operator==(rhs);
    }
    bool operator==(const Map2D& rhs) const {
        return W == rhs.W && H == rhs.H && data == rhs.data;
    }
};

std::ostream& operator<<(std::ostream&, const Map2D&);
