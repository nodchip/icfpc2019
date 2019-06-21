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
  int speed = (time_fast_wheels > 0) ? 2 : 1;
  Point p {wrappy};
  switch (c) {
  case UP:
    p.second += speed;
    break;
  case DOWN:
    p.second -= speed;
    break;
  case LEFT:
    p.first -= speed;
    break;
  case RIGHT:
    p.first += speed;
    break;
  }

  if (p.first < 0)
    p.first = 0;
  else if (p.first >= map[0].size())
    p.first = map[0].size() - 1;
  else if (p.second < 0)
    p.second = 0;
  else if (p.second >= map.size())
    p.second = map.size() - 1;

  // Update |map|. Need to simulate manipulators' behavior.
  map[wrappy.second][wrappy.first] = WRAPPED;
  wrappy = p;
  map[wrappy.second][wrappy.first] = WRAPPY;

  behave(c);
}

void Game::nop() {
  behave('Z');
}

// static const char CW = 'E';  // Clockwise
// static const char CCW = 'Q';  // Counterclockwise
void Game::turn(char c) {
  if (c == CW) {
    for (auto& manip : manipulators) {
      auto orig(manip);
      manip.first = orig.second;
      manip.second = -orig.first;
    }
  } else {
    for (auto& manip : manipulators) {
      auto orig(manip);
      manip.first = -orig.second;
      manip.second = orig.first;
    }
  }

  behave(c);
}

void Game::addManipulate(const Point& p) {
  assert (num_manipulators > 0);

  manipulators.push_back(p);
  --num_manipulators;

  std::ostringstream oss;
  oss << "B(" << p.first << "," << p.second << ")";
  behave(oss.str());
}

// static const char FAST = 'F';
// static const char DRILL = 'L';
void Game::useBooster(char c) {
  switch (c) {
  case FAST: {
    assert (fast_wheels > 0);
    --fast_wheels;
    time_fast_wheels = 50;
    break;
  }
  case DRILL: {
    --drills;
    time_drill = 30;
    break;
  }
  }

  behave(c);
}

void Game::behave(const char c) {
  behave(std::string(1, c));
}

void Game::behave(const std::string& behavior) {
  command += behavior;
  ++time;
  if (time_fast_wheels > 0) --time_fast_wheels;
  if (time_drill > 0) --time_drill;
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
