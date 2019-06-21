#pragma once

#include <string>
#include <vector>

#include "base.h"

const int DISTANCE_INF = 11451;

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
  std::vector<Direction> traj; // 最短で到達できるtraj  
};
