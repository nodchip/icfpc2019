#pragma once

// #include <iostream>
// #include <cstdio>
// #include <cstdint>
// #include <queue>
// #include <string>
// #include <thread>
// #include <chrono>
#include <unordered_map>

#include "base.h"
#include "booster.h"

namespace std {

template<>
class hash<Point> {
public:
  size_t operator()(const Point& p) const { return (p.x << 16) + p.y; }
};

}  // namespace std

namespace sampo {

constexpr int kMaxCostManipulator = 10;
constexpr int kMaxCostFastWheel = 5;

// Directions. Upper 4 bits is 'to', lower 4 bits is 'from'.
using Direction = uint8_t;

struct State {
  Point target = {-1, -1};
  std::unordered_map<Point, Direction> direction_map;
};

namespace Target {
static constexpr int kUnwrapped = 0;    // .
static constexpr int kManipulator = 1;  // B
static constexpr int kFastWheel = 2;    // F
static constexpr int kCloning = 3;      // C
static constexpr int kSpawn = 4;        // X
static constexpr int N = 5;
// static constexpr int kDrill = ;
// static constexpr int kBeacon = ;
};

constexpr Direction kUp    = 1;
constexpr Direction kDown  = 2;
constexpr Direction kLeft  = 3;
constexpr Direction kRight = 4;
constexpr Direction kFromMask = 0xf;
constexpr Direction kToMask = 0xf0;

inline Direction setFrom(const Direction dir, const Direction from) {
  return (dir & kToMask) | from;
}
inline Direction getFrom(const Direction& dir) {
  return dir & kFromMask;
}
inline Direction setTo(const Direction dir, const Direction to) {
  return (dir & kFromMask) | (to << 4);
}
inline Direction getTo(const Direction dir) {
  return (dir >> 4) & kFromMask;
}

inline bool isUnwrapped(int c) {
  return (c & (CellType::kObstacleBit | CellType::kWrappedBit)) == 0;
}

}  // namespace sampo
