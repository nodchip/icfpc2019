#include "game.h"

#include <ostream>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

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

Game::Game(const std::string& task) {
  char* p = const_cast<char*>(task.data());

  std::vector<Point> map_pos { ParseMap(p) };
  assert (*p == '#');
  wrappy = ParsePoint(++p);
  assert (*p == '#');
  std::vector<std::vector<Point>> obstacles { ParseObstacles(++p) };
  assert (*p == '#');
  std::vector<Booster> boosters { ParseBoosters(++p) };

  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
}

Game::Game(const std::vector<std::string>& mp)
  : map(mp) {
  std::reverse(map.begin(), map.end());
  wrappy = FindPoint(map);

  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
}

// static const char UP = 'W';
// static const char DOWN = 'S';
// static const char LEFT = 'A';
// static const char RIGHT = 'D';
void Game::move(char c) {
  command.push_back(c);
}

void Game::nop() {
  command.push_back('Z');
}

// static const char CW = 'E';  // Clockwise
// static const char CCW = 'Q';  // Counterclockwise
void Game::turn(char c) {
  command.push_back(c);
}

void Game::addManipulate(const Point& p) {
  std::ostringstream oss;
  oss << "B(" << p.first << "," << p.second << ")";
  command += oss.str();
}

// static const char FAST = 'F';
// static const char DRILL = 'L';
void Game::useBooster(char c) {
  command.push_back(c);
}

std::ostream& operator<<(std::ostream& os, const Game& game) {
  os << "Time: " << game.time << "\n";
  for (int i = game.map.size() - 1; i >= 0; --i)
    os << game.map[i] << "\n";

  os << "Boosters: B(" << game.num_manipulators << ") "
     << "F(" << game.fast_wheels << ") "
     << "L(" << game.drills << ")\n";
  if (game.time_fast_wheels > 0) {
    os << " Speedup (" << game.time_fast_wheels << ")\n";
  }
  if (game.time_drill > 0) {
    os << " Drill (" << game.time_drill << ")\n";
  }

  return os;
}
