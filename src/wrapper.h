#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <memory>

#include "base.h"
#include "map2d.h"

struct Action {
  Action(int timestamp_, bool fast_wheels_active_, bool drill_active_, Point before_pos, const std::vector<Point>& before_manipulator_offsets)
    : timestamp(timestamp_)
    , fast_wheels_active(fast_wheels_active_)
    , drill_active(drill_active_) {
    old_position = before_pos;
    new_position = before_pos;
    old_manipulator_offsets = before_manipulator_offsets;
    new_manipulator_offsets = before_manipulator_offsets;
  }
  int timestamp;
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
  std::vector<Point> pick_cloning;
  std::vector<Point> pick_teleport;
  // using boosters
  int use_manipulator = 0;
  int use_fast_wheel = 0;
  int use_drill = 0;
  int use_cloning = 0;
  int use_teleport = 0; // instaling, not teleporting.
  // active boosters
  bool fast_wheels_active = false;
  bool drill_active = false;
  // misc
  int spawned_index = -1;
};

struct Game;
struct Wrapper {
  Wrapper(Game* game_, Point pos_, int wrapper_spawn_index_);

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
  void teleport(const Point&);  // input: x,y
  static const char FAST = 'F';
  static const char DRILL = 'L';
  static const char RESET = 'R';
  void useBooster(char);  // input: FLR
  static const char CLONE = 'C';
  std::shared_ptr<Wrapper> cloneWrapper(); 

  Action getScaffoldAction();
  std::string getCommand() const; // command for this wrapper.
  bool undoAction(); // if no actions are stacked, fail and return false.

  Game* game;
  Map2D& map2d;
  Point pos;
  int index;
  std::vector<Action> actions;
  std::vector<Point> manipulators;

  // remained time of 'F'. While this is >0, speed becomes 2.
  int time_fast_wheels = 0;
  // remained time of 'L'. While this is >0, wrappy can go through obstacles.
  int time_drill = 0;

private:
  void moveAndPaint(Point p, Action& a);
  void doAction(Action a);
};
