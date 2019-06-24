#include "wrapper.h"

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
#include "game.h"

Wrapper::Wrapper(Game* game_, Point pos_, int wrapper_spawn_index_)
  : game(game_), pos(pos_), index(wrapper_spawn_index_), direction(Direction::D) {
  // initial manipulator position. it looks like the wrapper is facing right (D).
  //   *
  // @ *
  //   *
  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
}

Action Wrapper::getScaffoldAction() {
  // +1 for next action.
  Action a = {game->time + 1, time_fast_wheels > 0, time_drill > 0, pos, direction, manipulators};
  // pick boosters before move!
  pick(a);

  // If the last action is a fast move, pick a booster on its way.
  if (actions.size()) {
    // Last action.
    const Action& la = actions.back();
    if (la.fast_wheels_active &&
        la.old_position != la.new_position &&
        la.use_booster[BoosterType::TELEPORT] == 0) {
      const Point& op = la.old_position;
      const Point& np = la.new_position;
      game->pick(Point((op.x + np.x) / 2, (op.y + np.y) / 2), &a); // pick boosters before move!
    }
  }
  return a;
}

bool Wrapper::isMoveable(char c) {
  auto& map2d = game->map2d;

  {
    Point p {pos};
    switch (c) {
    case Action::UP:
      p.y += 1;
      break;
    case Action::DOWN:
      p.y -= 1;
      break;
    case Action::LEFT:
      p.x -= 1;
      break;
    case Action::RIGHT:
      p.x += 1;
      break;
    }

    if (!map2d.isInside(p)) {
      return false;
    }
    if ((map2d(p) & CellType::kObstacleBit) == 0) {
      return true;
    } else if (time_drill > 0) {
      return true;
    } else {
      return false;
    }
  }
}

bool Wrapper::move(char c) {
  auto& map2d = game->map2d;
  Action a = getScaffoldAction();
  a.command = c;

  {
    Point p {pos};
    switch (c) {
    case Action::UP:
      p.y += 1;
      break;
    case Action::DOWN:
      p.y -= 1;
      break;
    case Action::LEFT:
      p.x -= 1;
      break;
    case Action::RIGHT:
      p.x += 1;
      break;
    }

    if (!map2d.isInside(p)) {
      assert (false);
      return false;
    }
    if ((map2d(p) & CellType::kObstacleBit) == 0 ||
        time_drill > 0) {
      moveAndPaint(p, a);
    } else {
      assert (false);
      return false;
    }
  }

  if (time_fast_wheels > 0) {
    Point p {pos};
    switch (c) {
    case Action::UP:
      p.y += 1;
      break;
    case Action::DOWN:
      p.y -= 1;
      break;
    case Action::LEFT:
      p.x -= 1;
      break;
    case Action::RIGHT:
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

    if (map2d.isInside(p)) {
      if ((map2d(p) & CellType::kObstacleBit) == 0 ||
          time_drill > 0) {
        moveAndPaint(p, a);
      }
    }
  }

  a.new_position = pos;
  moveAndPaint(pos, a);
  doAction(a);
  return true;
}

void Wrapper::nop() {
  Action a = getScaffoldAction();
  a.command = "Z";
  doAction(a);
}

void Wrapper::turn(char c) {
  Action a = getScaffoldAction();
  a.command = c;

  if (c == Action::CW) {
    direction = turnCW(direction);
    for (auto& manip : manipulators) {
      auto orig(manip);
      manip.x = orig.y;
      manip.y = -orig.x;
    }
  } else {
    direction = turnCCW(direction);
    for (auto& manip : manipulators) {
      auto orig(manip);
      manip.x = -orig.y;
      manip.y = orig.x;
    }
  }

  moveAndPaint(pos, a);

  a.new_manipulator_offsets = manipulators;
  a.new_direction = direction;
  doAction(a);
}

bool Wrapper::addManipulator(const Point& p) {
  if (!canAddManipulator(p)) {
    assert (false);
    return false;
  }

  Action a = getScaffoldAction();
  assert (game->num_boosters[BoosterType::MANIPULATOR] > 0);

  manipulators.push_back(p);
  --game->num_boosters[BoosterType::MANIPULATOR];

  moveAndPaint(pos, a);

  std::ostringstream oss;
  oss << "B(" << p.x << "," << p.y << ")";

  a.use_booster[BoosterType::MANIPULATOR] += 1;
  a.new_manipulator_offsets = manipulators;
  a.command = oss.str();
  doAction(a);
  return true;
}

bool Wrapper::canAddManipulator(const Point& p) {
  // cannot place over an existing manipulator (or the wrapper itself).
  if (p == Point{0, 0}) return false;
  for (auto m : manipulators) {
    if (m == p) return false;
  }
  // it has to be 4-connected to existing manipulator (or the wrapper itself).
  for (auto dir : all_directions) {
    if (p + Point(dir) == Point{0, 0}) return true;
    for (auto m : manipulators) {
      if (p + Point(dir) == m) return true;
    }
  }
  return false;
}

bool Wrapper::teleport(const Point& p) {
  auto& map2d = game->map2d;
  Action a = getScaffoldAction();
  if ((map2d(p) & CellType::kTeleportTargetBit) == 0) {
    assert(false);
    return false;
  }

  std::ostringstream oss;
  oss << "T(" << p.x << "," << p.y << ")";

  moveAndPaint(p, a);

  a.command = oss.str();
  doAction(a);
  return true;
}

void Wrapper::pick(Action& a) {
  auto& map2d = game->map2d;
  assert (map2d.isInside(pos));
  game->pick(pos, &a);
}

void Wrapper::moveAndPaint(Point p, Action& a) {
  auto& map2d = game->map2d;
  assert (map2d.isInside(p));

  pos = p;
  game->paint(*this, &a);
}

bool Wrapper::useBooster(char c) {
  auto& map2d = game->map2d;
  Action a = getScaffoldAction();
  a.command = c;

  const int b = boosterFromChar(c).booster_type;
  if (game->num_boosters[b] <= 0) {
    assert (false);
    return false;
  }
  --game->num_boosters[b];
  a.use_booster[b] += 1;

  switch (c) {
  case Action::FAST: {
    time_fast_wheels += 50 + (time_fast_wheels ? 0 : 1); // rule specification has updated.
    break;
  }
  case Action::DRILL: {
    time_drill += 30 + (time_drill ? 0 : 1); // rule specification has updated.
    break;
  }
  case Action::BEACON: {
    map2d(pos) |= CellType::kTeleportTargetBit;
    break;
  }
  }

  doAction(a);
  return true;
}

Wrapper* Wrapper::cloneWrapper() {
  assert (game->num_boosters[BoosterType::CLONING] > 0);
  --game->num_boosters[BoosterType::CLONING];

  Action a = getScaffoldAction();
  a.command = "C";
  a.use_booster[BoosterType::CLONING] += 1;

  assert ((game->map2d(pos) & CellType::kSpawnPointBit) != 0);
  std::unique_ptr<Wrapper> new_wrapper(new Wrapper(game, pos, game->nextWrapperIndex())); 
  Wrapper* spawned = new_wrapper.get();
  a.spawned_index = spawned->index;
  game->addClonedWrapperForNextFrame(std::move(new_wrapper));

  doAction(a);
  return spawned;
}

std::string Wrapper::getCommand() const {
  std::ostringstream oss;
  for (auto& a : actions) {
    oss << a.command;
  }
  return oss.str();
}

bool Wrapper::undoAction() {
  auto& map2d = game->map2d;
  assert (!actions.empty());
  if (actions.empty()) return false;

  // recover the state.
  Action a = actions.back();
  actions.pop_back();
  // undo motion
  pos = a.old_position;
  direction = a.old_direction;
  // undo rotation and manipulator addition
  manipulators = a.old_manipulator_offsets;
  // undo paint
  for (auto p : a.absolute_new_wrapped_positions) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kWrappedBit) != 0);
    ++map2d.num_unwrapped;
    map2d(p) &= ~CellType::kWrappedBit;
  }
  // undo drill
  for (auto p : a.break_walls) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kObstacleBit) == 0);
    map2d(p) |= CellType::kObstacleBit;
  }
  for (auto booster : boosters) {
    // undo picking boosters (place boosters)
    for (auto p : a.pick_boosters[booster.booster_type]) {
      assert (map2d.isInside(p) && (map2d(p) & booster.map_bit) == 0);
      map2d(p) |= booster.map_bit;
      game->num_boosters[booster.booster_type] -= 1;
      assert (game->num_boosters[booster.booster_type] >= 0);
    }
    // undo using boosters
    game->num_boosters[booster.booster_type] += a.use_booster[booster.booster_type];
  }
  if (a.use_booster[BoosterType::FAST_WHEEL]) {
    time_fast_wheels -= 50; // rule specification has updated.
  }
  if (a.use_booster[BoosterType::DRILL]) {
    time_drill -= 30; // rule specification has updated.
  }
  // undo placing teleports
  if (a.use_booster[BoosterType::TELEPORT]) {
    assert (map2d.isInside(a.new_position) && (map2d(a.new_position) & CellType::kTeleportTargetBit) != 0);
    map2d(a.new_position) &= ~CellType::kTeleportTargetBit;
  }
  // undo time
  if (a.fast_wheels_active) { time_fast_wheels += 1; }
  if (a.drill_active) { time_drill += 1; }

  return true;
}

void Wrapper::doAction(Action a) {
  actions.push_back(a);
  if (time_fast_wheels > 0) --time_fast_wheels;
  if (time_drill > 0) --time_drill;
}
