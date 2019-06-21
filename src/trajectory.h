#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <cstdint>

#include "base.h"

const int DISTANCE_INF = 999999999;

enum class Direction : std::uint8_t { W, S, A, D };

struct Trajectory {
  Point from;                  // 元いた場所
  Point to;                    // 行き先
  int distance = DISTANCE_INF; // 距離
  bool use_drill = false;
  std::vector<Direction> path; // 最短で到達できるtraj
};

std::ostream &operator<<(std::ostream &, const Direction &);
std::ostream &operator<<(std::ostream &, const Trajectory &);
