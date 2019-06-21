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
#include "manipulator_reach.h"

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

Action Game::getScaffoldAction() {
  return {time_fast_wheels > 0, time_drill > 0, wrappy, manipulators};
}

// static const char UP = 'W';
// static const char DOWN = 'S';
// static const char LEFT = 'A';
// static const char RIGHT = 'D';
void Game::move(char c) {
  Action a = getScaffoldAction();
  a.command = c;

  int speed = (time_fast_wheels > 0) ? 2 : 1;

  for (int i = 0; i < speed; ++i) {
    Point p {wrappy};
    switch (c) {
    case UP:
      p.y += 1;
      break;
    case DOWN:
      p.y -= 1;
      break;
    case LEFT:
      p.x -= 1;
      break;
    case RIGHT:
      p.x += 1;
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
    assert (map2d.isInside(p));
    wrappy = p;
    if ((map2d(p) & CellType::kWrappedBit) == 0) {
      map2d(p) |= CellType::kWrappedBit;
      a.absolute_new_wrapped_positions.push_back(p);
    }

    // paint manipulator
    for (auto manip : absolutePositionOfReachableManipulators(map2d, wrappy, manipulators)) {
      if ((map2d(manip) & CellType::kWrappedBit) == 0) {
        a.absolute_new_wrapped_positions.push_back(manip);
        map2d(manip) |= CellType::kWrappedBit;
      }
    }

    // automatically pick up boosters with no additional time cost.
    if (map2d(p) & CellType::kBoosterManipulatorBit) {
      a.pick_manipulator.push_back(p);
      ++num_manipulators;
      map2d(p) &= ~CellType::kBoosterManipulatorBit;
    }
    if (map2d(p) & CellType::kBoosterFastWheelBit) {
      a.pick_fast_wheel.push_back(p);
      ++fast_wheels;
      map2d(p) &= ~CellType::kBoosterFastWheelBit;
    }
    if (map2d(p) & CellType::kBoosterDrillBit) {
      a.pick_drill.push_back(p);
      ++drills;
      map2d(p) &= ~CellType::kBoosterDrillBit;
    }
    if (map2d(p) & CellType::kBoosterTeleportBit) {
      a.pick_teleport.push_back(p);
      ++teleports;
      map2d(p) &= ~CellType::kBoosterTeleportBit;
    }
  }

  a.new_position = wrappy;
  doAction(a);
}

void Game::nop() {
  Action a = getScaffoldAction();
  a.command = "Z";
  doAction(a);
}

// static const char CW = 'E';  // Clockwise
// static const char CCW = 'Q';  // Counterclockwise
void Game::turn(char c) {
  Action a = getScaffoldAction();
  a.command = c;

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

  a.new_manipulator_offsets = manipulators;
  doAction(a);
}

void Game::addManipulate(const Point& p) {
  Action a = getScaffoldAction();
  assert (num_manipulators > 0);

  manipulators.push_back(p);
  --num_manipulators;

  std::ostringstream oss;
  oss << "B(" << p.x << "," << p.y << ")";

  a.use_manipulator += 1;
  a.new_manipulator_offsets = manipulators;
  a.command = oss.str();
  doAction(a);
}

// static const char FAST = 'F';
// static const char DRILL = 'L';
void Game::useBooster(char c) {
  Action a = getScaffoldAction();
  a.command = c;

  switch (c) {
  case FAST: {
    assert (fast_wheels > 0);
    --fast_wheels;
    time_fast_wheels = 50;
    a.use_fast_wheel += 1;
    break;
  }
  case DRILL: {
    --drills;
    time_drill = 30;
    a.use_drill += 1;
    break;
  }
  }

  doAction(a);
}

bool Game::undoAction() {
  assert (!actions.empty());
  if (actions.empty()) return false;

  // recover the state.
  Action a = actions.back();
  actions.pop_back();
  // undo motion
  wrappy = a.old_position;
  // undo rotation and manipulator addition
  manipulators = a.old_manipulator_offsets;
  // undo paint
  for (auto p : a.absolute_new_wrapped_positions) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kWrappedBit) != 0);
    map2d(p) &= ~CellType::kWrappedBit;
  }
  // undo drill
  for (auto p : a.break_walls) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kObstacleBit) == 0);
    map2d(p) |= CellType::kObstacleBit;
  }
  // place boosters
  for (auto p : a.pick_manipulator) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kBoosterManipulatorBit) == 0);
    map2d(p) |= CellType::kBoosterManipulatorBit;
  }
  for (auto p : a.pick_fast_wheel) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kBoosterFastWheelBit) == 0);
    map2d(p) |= CellType::kBoosterFastWheelBit;
  }
  for (auto p : a.pick_drill) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kBoosterDrillBit) == 0);
    map2d(p) |= CellType::kBoosterDrillBit;
  }
  for (auto p : a.pick_teleport) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kBoosterTeleportBit) == 0);
    map2d(p) |= CellType::kBoosterTeleportBit;
  }
  // undo using boosters
  num_manipulators += a.use_manipulator;
  fast_wheels += a.use_fast_wheel;
  drills += a.use_drill;
  teleports += a.use_teleport;
  // undo time
  time -= 1;
  if (a.fast_wheels_active) { time_fast_wheels += 1; }
  if (a.drill_active) { time_drill += 1; }

  return true;
}

std::string Game::getCommand() const {
  std::ostringstream oss;
  for (auto& a : actions) {
    oss << a.command;
  }
  return oss.str();
}

void Game::doAction(Action a) {
  actions.push_back(a);
  ++time;
  if (time_fast_wheels > 0) --time_fast_wheels;
  if (time_drill > 0) --time_drill;
}

std::ostream& operator<<(std::ostream& os, const Game& game) {
  os << "Time: " << game.time << "\n";
  for (auto& line : dumpMapString(game.map2d, game.wrappy)) {
    os << line << "\n";
  }

  os << "Boosters: B(" << game.num_manipulators << ") "
     << "F(" << game.fast_wheels << ") "
     << "L(" << game.drills << ") "
     << "R(" << game.teleports << ")\n";
  if (game.time_fast_wheels > 0) {
    os << " Speedup (" << game.time_fast_wheels << ")\n";
  }
  if (game.time_drill > 0) {
    os << " Drill (" << game.time_drill << ")\n";
  }

  return os;
}
