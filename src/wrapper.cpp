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
  : game(game_), map2d(game_->map2d), pos(pos_), index(wrapper_spawn_index_) {
  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
}

Action Wrapper::getScaffoldAction() {
  // +1 for next action.
  return {game->time + 1, time_fast_wheels > 0, time_drill > 0, pos, manipulators};
}

void Wrapper::move(char c) {
  Action a = getScaffoldAction();
  a.command = c;

  int speed = (time_fast_wheels > 0) ? 2 : 1;

  for (int i = 0; i < speed; ++i) {
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
      if ((map2d(p) & CellType::kObstacleBit) == 0) {
        moveAndPaint(p, a);
      } else if (time_drill > 0) {
        map2d(p) &= ~CellType::kObstacleBit;
        moveAndPaint(p, a);
      }
    }
  }

  a.new_position = pos;
  doAction(a);
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

  moveAndPaint(pos, a);

  a.new_manipulator_offsets = manipulators;
  doAction(a);
}

void Wrapper::addManipulate(const Point& p) {
  Action a = getScaffoldAction();
  assert (game->num_manipulators > 0);

  manipulators.push_back(p);
  --game->num_manipulators;

  moveAndPaint(pos, a);

  std::ostringstream oss;
  oss << "B(" << p.x << "," << p.y << ")";

  a.use_manipulator += 1;
  a.new_manipulator_offsets = manipulators;
  a.command = oss.str();
  doAction(a);
}

void Wrapper::teleport(const Point& p) {
  Action a = getScaffoldAction();

  std::ostringstream oss;
  oss << "T(" << p.x << "," << p.y << ")";

  moveAndPaint(p, a);

  a.command = oss.str();
  doAction(a);
}

void Wrapper::moveAndPaint(Point p, Action& a) {
  assert (map2d.isInside(p));

  pos = p;
  game->paint(*this, a);
}

void Wrapper::useBooster(char c) {
  Action a = getScaffoldAction();
  a.command = c;

  switch (c) {
  case Action::FAST: {
    assert (game->fast_wheels > 0);
    --game->fast_wheels;
    time_fast_wheels = 50;
    a.use_fast_wheel += 1;
    break;
  }
  case Action::DRILL: {
    assert (game->drills > 0);
    --game->drills;
    time_drill = 30;
    a.use_drill += 1;
    break;
  }
  case Action::BEACON: {
    --game->teleports;
    a.use_teleport += 1;
    map2d(pos) |= CellType::kTeleportTargetBit;
    break;
  }
  }

  doAction(a);
}

std::shared_ptr<Wrapper> Wrapper::cloneWrapper() {
  assert (game->clonings > 0);
  --game->clonings;

  Action a = getScaffoldAction();
  a.command = "C";
  a.use_cloning += 1;

  assert ((game->map2d(pos) & CellType::kBoosterUnknownXBit) != 0);
  auto spawned = std::make_shared<Wrapper>(game, pos, game->nextWrapperIndex());
  a.spawned_index = spawned->index;
  game->wrappers.push_back(spawned);

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
  assert (!actions.empty());
  if (actions.empty()) return false;

  // recover the state.
  Action a = actions.back();
  actions.pop_back();
  // undo motion
  pos = a.old_position;
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
  for (auto p : a.pick_cloning) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kBoosterCloningBit) == 0);
    map2d(p) |= CellType::kBoosterCloningBit;
  }
  for (auto p : a.pick_teleport) {
    assert (map2d.isInside(p) && (map2d(p) & CellType::kBoosterTeleportBit) == 0);
    map2d(p) |= CellType::kBoosterTeleportBit;
  }
  // undo using boosters
  game->num_manipulators += a.use_manipulator;
  game->fast_wheels += a.use_fast_wheel;
  game->drills += a.use_drill;
  game->clonings += a.use_cloning;
  game->teleports += a.use_teleport;
  // undo placing teleports
  if (a.use_teleport) {
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
