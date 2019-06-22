#include "game.h"

#include <iostream>
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

  auto w = std::make_shared<Wrapper>(this, parsed.wrappy, 0);
  Action dummy = w->getScaffoldAction();
  paint(*w, dummy);
  wrappers.push_back(w);
}

Game::Game(const std::vector<std::string>& mp) {
  ParsedMap parsed = parseMapString(mp);
  map2d = parsed.map2d;

  auto w = std::make_shared<Wrapper>(this, parsed.wrappy, 0);
  Action dummy = w->getScaffoldAction();
  paint(*w, dummy);
  wrappers.push_back(w);
}

bool Game::tick() {
  // make sure all wrappers has provided a command.
  for (auto& wrapper : wrappers) {
    assert (!wrapper->actions.empty());
    assert (wrapper->actions.back().timestamp == time + 1);
  }
  ++time;
  return true;
}


void Game::paint(const Wrapper& w, Action& a) {
  auto p = w.pos;
  assert (map2d.isInside(p));

  // paint & move
  if ((map2d(p) & CellType::kWrappedBit) == 0) {
    map2d(p) |= CellType::kWrappedBit;
    a.absolute_new_wrapped_positions.push_back(p);
  }

  // paint manipulator
  for (auto manip : absolutePositionOfReachableManipulators(map2d, p, w.manipulators)) {
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
  if (map2d(p) & CellType::kBoosterCloningBit) {
    a.pick_cloning.push_back(p);
    ++clonings;
    map2d(p) &= ~CellType::kBoosterCloningBit;
  }
  if (map2d(p) & CellType::kBoosterTeleportBit) {
    a.pick_teleport.push_back(p);
    ++teleports;
    map2d(p) &= ~CellType::kBoosterTeleportBit;
  }
}

bool Game::undo() {
  if (wrappers.empty()) return false;

  for (auto& w : wrappers) {
    w->undoAction();
  }

  // undo time
  time -= 1;
  return true;
}

std::string Game::getCommand() const {
  std::ostringstream oss;
  for (int i = 0; i < wrappers.size(); ++i) {
    oss << wrappers[i]->getCommand();
    if (i + 1 < wrappers.size()) {
      oss << '#';
    }
  }
  return oss.str();
}

std::vector<Point> Game::getWrapperPositions() const {
  std::vector<Point> wrapper_positions;
  for (auto& w : wrappers) {
    wrapper_positions.push_back(w->pos);
  }
  return wrapper_positions;
}

std::ostream& operator<<(std::ostream& os, const Game& game) {

  os << "Time: " << game.time << "\n";
  for (auto& line : dumpMapString(game.map2d, game.getWrapperPositions())) {
    os << line << "\n";
  }

  os << "Boosters: B(" << game.num_manipulators << ") "
     << "F(" << game.fast_wheels << ") "
     << "L(" << game.drills << ") "
     << "C(" << game.clonings << ") "
     << "R(" << game.teleports << ")\n";
  os << "Wrappers: " << game.wrappers.size() << "\n";
  for (auto& w : game.wrappers) {
    os << w->index << " : ";
    if (w->time_fast_wheels > 0) {
      os << " Speedup (" << w->time_fast_wheels << ")\n";
    }
    if (w->time_drill > 0) {
      os << " Drill (" << w->time_drill << ")\n";
    }
    os << "\n";
  }

  return os;
}
