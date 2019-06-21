
#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "base.h"

namespace CellType {
  static constexpr int kEmpty = 0;
  static constexpr int kWrappedBit = 1 << 0;
  static constexpr int kObstacleBit = 1 << 1;
  static constexpr int kBoosterManipulatorBit = 1 << 2; // B
  static constexpr int kBoosterFastWheelBit = 1 << 3; // F
  static constexpr int kBoosterDrillBit = 1 << 4; // L
  static constexpr int kBoosterUnknownXBit = 1 << 5; // X
  static constexpr int kBoosterTeleportBit = 1 << 6; // R
  static constexpr int kTeleportTargetBit = 1 << 7; // installed teleport target
}

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
    T& operator()(Point p) { return data[p.y * W + p.x]; }
    T operator()(Point p) const { return data[p.y * W + p.x]; }
    bool operator!=(const Map2D& rhs) const {
        return !operator==(rhs);
    }
    bool operator==(const Map2D& rhs) const {
        return W == rhs.W && H == rhs.H && data == rhs.data;
    }
};

std::ostream& operator<<(std::ostream&, const Map2D&);