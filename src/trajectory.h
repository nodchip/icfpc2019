#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <cstdint>

#include "base.h"

const int DISTANCE_INF = 999999999;

enum class Direction : std::uint8_t { W, S, A, D };

struct Trajectory {
  Point origin;                // 元いた場所
  Point from;                  // 元いた場所から最短経路で来る場合、どのマスから来るか
  Point to;                    // 行き先
  int distance = DISTANCE_INF; // 距離
  int eval = DISTANCE_INF; // 評価値(低いほどよい)
  bool use_drill = false;
  std::vector<Direction> path; // 最短で到達できるtraj
};

std::ostream &operator<<(std::ostream &, const Direction &);
std::ostream &operator<<(std::ostream &, const Trajectory &);

char Direction2Char(const Direction d);
