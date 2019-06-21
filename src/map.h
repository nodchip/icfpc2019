#pragma once

#include <string>
#include <vector>

#include "base.h"

// '.': non wrapped
// ' ': wrapped
// 'B': Booster
// 'F': Fast Wheel
// 'L': Drill
// 'X': Something
// '#': Wall

struct Map {
  Map(const std::string& desc);
  Map(const std::vector<std::string>& map);

  std::vector<std::string> map;
  Point wrappy_point;
  int time = 0;
};
