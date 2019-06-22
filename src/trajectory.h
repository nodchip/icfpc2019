#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <cstdint>

#include "base.h"

const int DISTANCE_INF = 999999999;

enum class Direction : std::uint8_t { W, S, A, D };

struct Trajectory {
  Direction last_move;         // 最後にしたmove
  Point pos;                   // 自分の位置
  int distance = DISTANCE_INF; // 距離
  bool use_drill = false;      // 到達するまでにdrillを使ったか
};

std::ostream &operator<<(std::ostream &, const Direction &);
std::ostream &operator<<(std::ostream &, const Trajectory &);

char Direction2Char(const Direction d);
