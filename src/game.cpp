#include "game.h"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <ostream>
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

  auto w = std::make_unique<Wrapper>(this, parsed.wrappy, 0);
  pick(*w, nullptr);
  paint(*w, nullptr);
  wrappers.push_back(std::move(w));
}

Game::Game(const std::vector<std::string>& mp) : Game() {
  ParsedMap parsed = parseMapString(mp);
  map2d = parsed.map2d;

  auto w = std::make_unique<Wrapper>(this, parsed.wrappy, 0);
  pick(*w, nullptr);
  paint(*w, nullptr);
  wrappers.push_back(std::move(w));
}

bool Game::tick() {
  // make sure all wrappers has provided a command.
#ifndef NDEBUG
  for (auto& wrapper : wrappers) {
    assert (!wrapper->actions.empty());
    assert (wrapper->actions.back().timestamp == time + 1);
  }
#endif  // !defined(NDEBUG)
  ++time;
  // add new wrappers.
  for (auto&& w : next_wrappers) {
    wrappers.push_back(std::move(w));
  }
  next_wrappers.clear();
  return true;
}

void Game::pick(const Wrapper& w, Action* a_optional) {
  // automatically pick up boosters with no additional time cost.
  for (auto booster : boosters) {
    if (map2d(w.pos) & booster.map_bit) {
      if (a_optional) {
        assert (booster.booster_type < a_optional->pick_boosters.size());
        a_optional->pick_boosters[booster.booster_type].push_back(w.pos);
      }
      assert (booster.booster_type < num_boosters.size());
      ++num_boosters[booster.booster_type];
      map2d(w.pos) &= ~booster.map_bit;
    }
  }
}

void Game::paint(const Wrapper& w, Action* a_optional) {
  auto p = w.pos;
  assert (map2d.isInside(p));

  // paint
  if ((map2d(p) & CellType::kWrappedBit) == 0) {
    map2d(p) |= CellType::kWrappedBit;
    --map2d.num_unwrapped;
    if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(p);
  }

  // paint manipulator
  for (auto manip : absolutePositionOfReachableManipulators(map2d, p, w.manipulators)) {
    if ((map2d(manip) & CellType::kWrappedBit) == 0) {
      if (a_optional) a_optional->absolute_new_wrapped_positions.push_back(manip);
      map2d(manip) |= CellType::kWrappedBit;
      --map2d.num_unwrapped;
    }
  }
}

bool Game::undo() {
  if (time <= 0) return false;
  if (wrappers.empty()) return false;

  for (auto it = wrappers.begin(); it != wrappers.end();) {
    (*it)->undoAction();
    // unspawn.
    if ((*it)->actions.empty()) {
      it = wrappers.erase(it);
    } else {
      ++it;
    }
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

bool Game::isEnd() const {
  return countUnwrapped() == 0;
}

std::vector<Point> Game::getWrapperPositions() const {
  std::vector<Point> wrapper_positions;
  for (auto& w : wrappers) {
    wrapper_positions.push_back(w->pos);
  }
  return wrapper_positions;
}

void Game::addClonedWrapperForNextFrame(std::unique_ptr<Wrapper> wrapper) { 
  next_wrappers.push_back(std::move(wrapper));
}

std::ostream& operator<<(std::ostream& os, const Game& game) {
  os << "Time: " << game.time << "\n";
  for (auto& line : dumpMapString(game.map2d, game.getWrapperPositions())) {
    os << line << "\n";
  }
  os << "Unwrapped: " << game.map2d.num_unwrapped << "\n";

  os << "Boosters: B(" << game.num_boosters[BoosterType::MANIPULATOR] << ") "
     << "F(" << game.num_boosters[BoosterType::FAST_WHEEL] << ") "
     << "L(" << game.num_boosters[BoosterType::DRILL] << ") "
     << "C(" << game.num_boosters[BoosterType::CLONING] << ") "
     << "R(" << game.num_boosters[BoosterType::TELEPORT] << ")\n";
  os << "Wrappers: " << game.wrappers.size() << "\n";
  for (auto& w : game.wrappers) {
    os << w->index << " : ";
    os << "Dir[";
    switch (w->direction) {
      case Direction::W: os << "↑"; break;
      case Direction::A: os << "←"; break;
      case Direction::S: os << "↓"; break;
      case Direction::D: os << "→"; break;
    }
    os << "]";
    if (w->time_fast_wheels > 0) {
      os << " Speedup (" << w->time_fast_wheels << ")\n";
    }
    if (w->time_drill > 0) {
      os << " Drill (" << w->time_drill << ")\n";
    }
    os << "\n";
  }
  os << "Commad: " << game.getCommand() << "\n";

  return os;
}
