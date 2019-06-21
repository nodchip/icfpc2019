#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "base.h"
#include "game_map.h"

struct Action {
  Action(bool fast_wheels_active_, bool drill_active_, Point before_pos, const std::vector<Point>& before_manipulator_offsets) {
    old_position = before_pos;
    new_position = before_pos;
    old_manipulator_offsets = before_manipulator_offsets;
    new_manipulator_offsets = before_manipulator_offsets;
  }
  // old state
  Point old_position;
  std::vector<Point> old_manipulator_offsets;
  // command string
  std::string command;
  // motion (including teleport)
  Point new_position;
  // wrapped (only new ones)
  std::vector<Point> absolute_new_wrapped_positions;
  // manipulators
  std::vector<Point> new_manipulator_offsets;
  // break walls
  std::vector<Point> break_walls;
  // picking boosters at
  std::vector<Point> pick_manipulator;
  std::vector<Point> pick_fast_wheel;
  std::vector<Point> pick_drill;
  std::vector<Point> pick_teleport;
  // using boosters
  int use_manipulator = 0;
  int use_fast_wheel = 0;
  int use_drill = 0;
  int use_teleport = 0; // instaling, not teleporting.
  // active boosters
  bool fast_wheels_active = false;
  bool drill_active = false;
};

struct Game {
  Game() = default;
  Game(const std::string& desc); // initialize using a task description string from *.desc file.
  Game(const std::vector<std::string>& map); // initialize by a raster *.map file.

  // Control Wrappy
  static const char UP = 'W';
  static const char DOWN = 'S';
  static const char LEFT = 'A';
  static const char RIGHT = 'D';
  void move(char);  // input: WSAD
  void nop();  // input: Z
  static const char CW = 'E';  // Clockwise
  static const char CCW = 'Q';  // Counterclockwise
  void turn(char);  // input: EQ
  void addManipulate(const Point&);  // input: x,y
  static const char FAST = 'F';
  static const char DRILL = 'L';
  void useBooster(char);  // input: FL

  Action getScaffoldAction();
  bool undo(); // if no actions are stacked, fail and return false.

  // State of Game
  std::vector<Action> actions;
  std::string command;
  int time = 0;


  // refactored map representation
  Map2D map2d;

  // State of Wrappy ===================================
  Point wrappy;
  std::vector<Point> manipulators;

  // Boosters
  int num_manipulators = 0;  // number of unused manipulators
  int fast_wheels = 0;  // Increase if take 'F'
  int drills = 0;  // Increase if take 'L'
  int teleports = 0;  // Increase if take 'R'
  // remained time of 'F'. While this is >0, speed becomes 2.
  int time_fast_wheels = 0;
  // remained time of 'L'. While this is >0, wrappy can go through obstacles.
  int time_drill = 0;

private:
  void behave(const char c);
  void behave(const std::string& behavior);
};

// Outputs game status
std::ostream& operator<<(std::ostream&, const Game&);
