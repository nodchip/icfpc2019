#pragma once

#include <ostream>
#include <vector>

using Point = std::pair<int, int>;
using Polygon = std::vector<Point>;

struct Map2D {
    using T = int;
    int W = 0;
    int H = 0;
    std::vector<T> data;

    Map2D(int W_, int H_) : W(std::max(W_, 0)), H(std::max(H_, 0)) {
        data.assign(W * H, 0);
    }

    T& operator()(int x, int y) { return data[y * W + x]; }
    T operator()(int x, int y) const { return data[y * W + x]; }
};

std::ostream& operator<<(std::ostream&, const Map2D&);
