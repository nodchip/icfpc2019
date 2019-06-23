#pragma once

#include "game.h"

namespace utils {
  int countUnWrappedArea(const Game &game, const Point &from);
  std::vector<std::vector<double>> getGloryMap(const Game &game);
  void processCurrentGloryMap(const Game &game, const std::vector<std::vector<double>> &iMap, std::vector<std::vector<double>> &oMap);
  std::vector<Point> findNearestWay(const Game &game, const Point &from, const Point &to);
} // namespace utils