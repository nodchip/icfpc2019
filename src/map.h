#pragma once

#include <string>
#include <vector>

// '.': non wrapped
// ' ': wrapped
// 'B': Booster
// 'F': Fast Wheel
// 'L': Drill
// 'X': Something
// '#': Wall

class Map {
public:
  Map(const std::string& desc);

  const std::vector<std::string>& dump() const { return map_; }

  std::vector<std::string> map_;
};
