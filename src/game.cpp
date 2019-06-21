#include "game.h"

#include <ostream>
#include <algorithm>
#include <limits>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "fill_polygon.h"


Game::Game(const std::string& task) {
  ParsedMap parsed = parseDescString(task);
  map2d = parsed.map2d;
  wrappy = parsed.wrappy;

  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
}

Game::Game(const std::vector<std::string>& mp) {
  ParsedMap parsed = parseMapString(mp);
  map2d = parsed.map2d;
  wrappy = parsed.wrappy;
  
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
    p.y += speed;
    break;
  case DOWN:
    p.y -= speed;
    break;
  case LEFT:
    p.x -= speed;
    break;
  case RIGHT:
    p.x += speed;
    break;
  }

  if (p.x < 0)
    p.x = 0;
  else if (p.x >= map2d.W)
    p.x = map2d.W - 1;
  else if (p.y < 0)
    p.y = 0;
  else if (p.y >= map2d.H)
    p.y = map2d.H - 1;

  // paint & move
  wrappy = p;
  map2d(p) |= CellType::kWrappedBit;

  // automatically pick up boosters with no additional time cost.
  if (map2d(p) & CellType::kBoosterManipulatorBit) {
    ++num_manipulators;
    map2d(p) &= ~CellType::kBoosterManipulatorBit;
  }
  if (map2d(p) & CellType::kBoosterFastWheelBit) {
    ++fast_wheels;
    map2d(p) &= ~CellType::kBoosterFastWheelBit;
  }
  if (map2d(p) & CellType::kBoosterDrillBit) {
    ++drills;
    map2d(p) &= ~CellType::kBoosterDrillBit;
  }
  if (map2d(p) & CellType::kBoosterTeleportBit) {
    ++teleports;
    map2d(p) &= ~CellType::kBoosterTeleportBit;
  }

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
      manip.x = orig.y;
      manip.y = -orig.x;
    }
  } else {
    for (auto& manip : manipulators) {
      auto orig(manip);
      manip.x = -orig.y;
      manip.y = orig.x;
    }
  }

  behave(c);
}

void Game::addManipulate(const Point& p) {
  assert (num_manipulators > 0);

  manipulators.push_back(p);
  --num_manipulators;

  std::ostringstream oss;
  oss << "B(" << p.x << "," << p.y << ")";
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


std::vector<std::string> Game::createMap() const {
  std::vector<std::vector<char>> charmap;

  for (int y = 0; y < map2d.H; ++y) {
    std::vector<char> line(map2d.W, WALL);
    for (int x = 0; x < map2d.W; ++x) {
      char c = NON_WRAPPED;
      if (map2d(x, y) & CellType::kWrappedBit) { c = WRAPPED; }
      if (map2d(x, y) & CellType::kBoosterManipulatorBit) { c = BOOSTER_MANIPULATOR; }
      if (map2d(x, y) & CellType::kBoosterFastWheelBit) { c = BOOSTER_FAST_WHEEL; }
      if (map2d(x, y) & CellType::kBoosterDrillBit) { c = BOOSTER_DRILL; }
      if (map2d(x, y) & CellType::kBoosterUnknownXBit) { c = UNKNOWN; }
      if (map2d(x, y) & CellType::kBoosterTeleportBit) { c = BOOSTER_TELEPORT; }
      if (map2d(x, y) & CellType::kObstacleBit) { c = WALL; } // highest priority
      line[x] = c;
    }
    charmap.push_back(line);
  }

  charmap[wrappy.y][wrappy.x] = WRAPPY;

  std::vector<std::string> result;
  for (auto& line : charmap) {
    line.push_back('\0');
    result.push_back(std::string(line.begin(), line.end()));
  }
  return result;
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
  auto map = game.createMap();
  os << "Time: " << game.time << "\n";
  for (int i = map.size() - 1; i >= 0; --i)
    os << map[i] << "\n";

  os << "Boosters: B(" << game.num_manipulators << ") "
     << "F(" << game.fast_wheels << ") "
     << "L(" << game.drills << ")"
     << "R(" << game.teleports << ")\n";
  if (game.time_fast_wheels > 0) {
    os << " Speedup (" << game.time_fast_wheels << ")\n";
  }
  if (game.time_drill > 0) {
    os << " Drill (" << game.time_drill << ")\n";
  }

  return os;
}
