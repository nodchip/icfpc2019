#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "base.h"

const int DISTANCE_INF = 999999999;

enum Direction { W = 0, S = 1, A = 2, D = 3 };

struct Trajectory {
  Point from;                  // 元いた場所
  Point to;                    // 行き先
  int distance = DISTANCE_INF; // 距離
  bool use_dig;
  std::vector<Direction> path; // 最短で到達できるtraj
};

std::ostream &operator<<(std::ostream &, const Trajectory &);
