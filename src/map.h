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

  Point wrappy_point;
  int time = 0;

  // To display a map, use ostream::operator<<.
  // Y direction maybe wrong.
  std::vector<std::string> map;
};

std::ostream& operator<<(std::ostream&, const Map&);
