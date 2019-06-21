#pragma once

#include <string>
#include <vector>
#include <ostream>

#include "base.h"

const int DISTANCE_INF = 32767;

enum Direction{
  W = 0,
  S = 1,
  A = 2,
  D = 3
};

struct Trajectory{
  Trajectory(){
    distance = DISTANCE_INF;
  }

  Point from; // 元いた場所
  Point to; // 行き先
  int distance; // 距離
  bool use_dig;
  std::vector<Direction> path; // 最短で到達できるtraj  
};

std::ostream& operator<<(std::ostream&, const Trajectory&);
