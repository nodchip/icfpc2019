#pragma once

#include <array>
#include <vector>
#include <memory>

#include "base.h"
#include "booster.h"

struct Action {
  Action(int timestamp_, bool fast_wheels_active_, bool drill_active_, Point before_pos, Direction before_dir, const std::vector<Point>& before_manipulator_offsets)
    : timestamp(timestamp_)
    , fast_wheels_active(fast_wheels_active_)
    , drill_active(drill_active_) {
    old_position = before_pos;
    new_position = before_pos;
    old_direction = before_dir;
    new_direction = before_dir;
    old_manipulator_offsets = before_manipulator_offsets;
    new_manipulator_offsets = before_manipulator_offsets;
    for (int i = 0; i < BoosterType::N; ++i) {
      pick_boosters[i] = {};
      use_booster[i] = 0;
    }
  }
  int timestamp;
  // old state
  Point old_position;
  Direction old_direction;
  std::vector<Point> old_manipulator_offsets;
  // command string
  std::string command;
  // motion (including teleport)
  Point new_position;
  Direction new_direction;
  // wrapped (only new ones)
  std::vector<Point> absolute_new_wrapped_positions;
  // manipulators
  std::vector<Point> new_manipulator_offsets;
  // break walls
  std::vector<Point> break_walls;
  // picking boosters at
  std::array<std::vector<Point>, BoosterType::N> pick_boosters;
  // using boosters
  std::array<int, BoosterType::N> use_booster; // NOTE: user_booster[TELEPORT] means installing, not teleporting.
  // active boosters
  bool fast_wheels_active = false;
  bool drill_active = false;
  // misc
  int spawned_index = -1;

  // Action command
  static const char UP = 'W';
  static const char DOWN = 'S';
  static const char LEFT = 'A';
  static const char RIGHT = 'D';
  static const char CW = 'E';  // Clockwise
  static const char CCW = 'Q';  // Counterclockwise
  static const char FAST = 'F';
  static const char DRILL = 'L';
  static const char BEACON = 'R';
  static const char CLONE = 'C';
};
