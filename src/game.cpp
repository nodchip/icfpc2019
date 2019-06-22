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

Game::Game() {
  for (int i = 0; i < BoosterType::N; ++i) {
    num_boosters[i] = {};
  }
}

Game::Game(const std::string& task) : Game() {
  ParsedMap parsed = parseDescString(task);
  map2d = parsed.map2d;

  auto w = std::make_shared<Wrapper>(this, parsed.wrappy, 0);
  paint(*w, nullptr);
  wrappers.push_back(w);
}

Game::Game(const std::vector<std::string>& mp) : Game() {
  ParsedMap parsed = parseMapString(mp);
  map2d = parsed.map2d;

  auto w = std::make_shared<Wrapper>(this, parsed.wrappy, 0);
  paint(*w, nullptr);
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


void Game::paint(const Wrapper& w, Action* a_optional) {
  auto p = w.pos;
  assert (map2d.isInside(p));

  // paint & move
  if ((map2d(p) & CellType::kWrappedBit) == 0) {
    map2d(p) |= CellType::kWrappedBit;
    if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(p);
  }

  // paint manipulator
  for (auto manip : absolutePositionOfReachableManipulators(map2d, p, w.manipulators)) {
    if ((map2d(manip) & CellType::kWrappedBit) == 0) {
      if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(manip);
      map2d(manip) |= CellType::kWrappedBit;
    }
  }

  // automatically pick up boosters with no additional time cost.
  for (auto booster : boosters) {
    if (map2d(p) & booster.map_bit) {
      if (a_optional) {
        assert (booster.booster_type < a_optional->pick_boosters.size());
        a_optional->pick_boosters[booster.booster_type].push_back(p);
      }
      assert (booster.booster_type < num_boosters.size());
      ++num_boosters[booster.booster_type];
      map2d(p) &= ~booster.map_bit;
    }
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

  os << "Boosters: B(" << game.num_boosters[BoosterType::MANIPULATOR] << ") "
     << "F(" << game.num_boosters[BoosterType::FAST_WHEEL] << ") "
     << "L(" << game.num_boosters[BoosterType::DRILL] << ") "
     << "C(" << game.num_boosters[BoosterType::CLONING] << ") "
     << "R(" << game.num_boosters[BoosterType::TELEPORT] << ")\n";
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
