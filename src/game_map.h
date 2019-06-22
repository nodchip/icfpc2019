
#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <cassert>

#include "base.h"

#define MAP2D_CHECK_INSIDE
#ifdef MAP2D_CHECK_INSIDE
# define MAP2D_ASSERT(eq) assert(eq)
#else
# define MAP2D_ASSERT(eq) 
#endif

namespace CellType {
  static constexpr int kEmpty = 0;
  static constexpr int kWrappedBit = 1 << 0;
  static constexpr int kObstacleBit = 1 << 1;
  static constexpr int kBoosterManipulatorBit = 1 << 2; // B
  static constexpr int kBoosterFastWheelBit = 1 << 3; // F
  static constexpr int kBoosterDrillBit = 1 << 4; // L
  static constexpr int kBoosterUnknownXBit = 1 << 5; // X
  static constexpr int kBoosterTeleportBit = 1 << 6; // R
  static constexpr int kBoosterCloningBit = 1 << 7; // C
  static constexpr int kTeleportTargetBit = 1 << 8; // installed teleport target
}

// character representation of map ======================================
static const char NON_WRAPPED = '.';
static const char WRAPPED = ' ';
static const char WRAPPY = '@';
static const char BOOSTER_MANIPULATOR = 'B';
static const char BOOSTER_FAST_WHEEL = 'F';
static const char BOOSTER_DRILL = 'L';
static const char BOOSTER_TELEPORT = 'R';
static const char BOOSTER_CLONING = 'C';
static const char WALL = '#';
static const char UNKNOWN = 'X';

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
        assert (vals.size() == W * H);
        auto it = vals.begin();
        for (int y = 0; y < H; ++y) {
            for (int x = 0; x < W; ++x, ++it) {
                (*this)(x, y) = *it;
            }
        }
    }

    T& operator()(int x, int y) { MAP2D_ASSERT(isInside(x, y)); return data[y * W + x]; }
    T operator()(int x, int y) const { MAP2D_ASSERT(isInside(x, y)); return data[y * W + x]; }
    T& operator()(Point p) { MAP2D_ASSERT(isInside(p)); return data[p.y * W + p.x]; }
    T operator()(Point p) const { MAP2D_ASSERT(isInside(p)); return data[p.y * W + p.x]; }
    bool operator!=(const Map2D& rhs) const {
        return !operator==(rhs);
    }
    bool operator==(const Map2D& rhs) const {
        return W == rhs.W && H == rhs.H && data == rhs.data;
    }
    bool isInside(int x, int y) const {
        return 0 <= x && x < W && 0 <= y && y < H;
    }
    bool isInside(Point p) const {
        return isInside(p.x, p.y);
    }
};

using Booster = std::pair<char, Point>;

struct ParsedMap {
    Map2D map2d;
    Point wrappy;
};
// parse *.desc string to construct Map2D and obtain other info.
ParsedMap parseDescString(std::string desc_string);

// parse *.map string to construct Map2D and obtain other info.
// map_strings_top_to_bottom[H - 1 - y] corresponds to the y-line.
ParsedMap parseMapString(std::vector<std::string> map_strings_top_to_bottom);

// dump map string for display. the first row corresponds to the highest y.
// do not contain line terminator at the end of lines.
std::vector<std::string> dumpMapString(const Map2D& map2d, std::vector<Point> wrappy_list);

std::ostream& operator<<(std::ostream&, const Map2D&);
