#include "map.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

using Point = std::pair<int, int>;
using Booster = std::pair<char, Point>;

namespace {

Point ParsePoint(char*& p) {
  assert (*p == '(');
  Point pos;
  int x = std::strtol(++p, &p, 10);
  assert (*p == ',');
  int y = std::strtol(++p, &p, 10);
  assert (*p == ')');
  ++p;

  return {x, y};
}

std::vector<Point> ParseMap(char*& p) {
  std::vector<Point> map_pos;

  assert (*p == '(');
  map_pos.emplace_back(ParsePoint(p));
  while (*p == ',') {
    map_pos.emplace_back(ParsePoint(++p));
  }

  return map_pos;
}

std::vector<std::vector<Point>> ParseObstacles(char*& p) {
  std::vector<std::vector<Point>> obstacles;
  assert (*p == '(');
  obstacles.emplace_back(ParseMap(p));
  while (*p == ';') {
    obstacles.emplace_back(ParseMap(++p));
  }
  return obstacles;
}

Booster ParseBooster(char*& p) {
  assert (std::strchr("BFLX", *p) != nullptr);
  char code = *p;
  Point point { ParsePoint(++p) };
  return {code, point};
}

std::vector<Booster> ParseBoosters(char*& p) {
  assert (std::strchr("BFLX", *p) != nullptr);
  std::vector<Booster> boosters;
  boosters.emplace_back(ParseBooster(p));
  while (*p == ';') {
    boosters.emplace_back(ParseBooster(++p));
  }
  return boosters;
}

Point FindPoint(const std::vector<std::string>& map) {
  for (int y = 0; y < map.size(); ++y) {
    for (int x = 0; x < map[y].size(); ++x) {
      if (map[y][x] == '@')
        return {x, y};
    }
  }
  return {-1, -1};
}

}  // namespace

Map::Map(const std::string& task) {
  char* p = const_cast<char*>(task.data());

  std::vector<Point> map_pos { ParseMap(p) };
  assert (*p == '#');
  wrappy_point = ParsePoint(++p);
  assert (*p == '#');
  std::vector<std::vector<Point>> obstacles { ParseObstacles(++p) };
  assert (*p == '#');
  std::vector<Booster> boosters { ParseBoosters(++p) };
}

Map::Map(const std::vector<std::string>& mp)
  : map(mp) {
  wrappy_point = FindPoint(map);
}

