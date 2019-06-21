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

  BoundingBox map_bbox = calcBoundingBox(map_pos);
  assert (map_bbox.lower.x >= 0);
  assert (map_bbox.lower.y >= 0);
  assert (map_bbox.isValid());
  map2d = Map2D(map_bbox.upper.x, map_bbox.upper.y, CellType::kObstacle);
  FillPolygon(map2d, map_pos, CellType::kEmpty);
  for (const auto& obstacle : obstacles) {
    FillPolygon(map2d, obstacle, CellType::kObstacle);
  }

  for (auto booster : boosters) {
    switch (booster.first) {
      case BOOSTER_MANIPULATOR:
        placed_booster_manipulators.push_back(booster.second);
        break;
      case BOOSTER_FAST_WHEEL:
        placed_booster_fast_wheels.push_back(booster.second);
        break;
      case BOOSTER_DRILL:
        placed_booster_drills.push_back(booster.second);
        break;
      case UNKNOWN:
        // actually not a booster but a part of the map.
        map2d(booster.second.x, booster.second.y) = CellType::kUnknownX;
    }
  }

  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
}

Game::Game(const std::vector<std::string>& mp) {
  std::vector<std::string> maplines = mp;
  
  std::reverse(maplines.begin(), maplines.end());
  wrappy = FindPoint(maplines);

  int H = maplines.size();
  int W = maplines[0].size();
  map2d = Map2D(W, H, CellType::kObstacle);
  for (int y = 0; y < H; ++y) {
    for (int x = 0; x < W; ++x) {
      switch (maplines[y][x]) {
        case NON_WRAPPED:
          map2d(x, y) = CellType::kEmpty;
          break;
        case WRAPPED:
          map2d(x, y) = CellType::kWrapped;
          break;
        case WRAPPY:
          map2d(x, y) = CellType::kEmpty;
          wrappy = {x, y};
          break;
        case BOOSTER_MANIPULATOR:
          map2d(x, y) = CellType::kEmpty;
          placed_booster_manipulators.push_back({x, y});
          break;
        case BOOSTER_FAST_WHEEL:
          map2d(x, y) = CellType::kEmpty;
          placed_booster_fast_wheels.push_back({x, y});
          break;
        case BOOSTER_DRILL:
          map2d(x, y) = CellType::kEmpty;
          placed_booster_drills.push_back({x, y});
          break;
        case WALL:
          map2d(x, y) = CellType::kObstacle;
          break;
        case UNKNOWN:
          map2d(x, y) = CellType::kUnknownX;
          break;
        default:
          assert(false);
          break;
      }
    }
  }

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

  // no more required.
  // // Update |map|. Need to simulate manipulators' behavior.
  // map[wrappy.y][wrappy.x] = WRAPPED;
  // wrappy = p;
  // map[wrappy.y][wrappy.x] = WRAPPY;

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
      switch (map2d(x, y)) {
        case CellType::kEmpty:
          line[x] = NON_WRAPPED;
          break;
        case CellType::kWrapped:
          line[x] = WRAPPED;
          break;
        case CellType::kObstacle:
          line[x] = WALL;
          break;
        case CellType::kUnknownX:
          line[x] = UNKNOWN;
          break;
        default:
          line[x] = '?';
          break;
      }
    }
    charmap.push_back(line);
  }

  for (auto& B : placed_booster_manipulators) {
    charmap[B.y][B.x] = BOOSTER_MANIPULATOR;
  }
  for (auto& F : placed_booster_fast_wheels) {
    charmap[F.y][F.x] = BOOSTER_FAST_WHEEL;
  }
  for (auto& L : placed_booster_drills) {
    charmap[L.y][L.x] = BOOSTER_DRILL;
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
     << "L(" << game.drills << ")\n";
  if (game.time_fast_wheels > 0) {
    os << " Speedup (" << game.time_fast_wheels << ")\n";
  }
  if (game.time_drill > 0) {
    os << " Drill (" << game.time_drill << ")\n";
  }

  return os;
}
