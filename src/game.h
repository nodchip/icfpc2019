#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "base.h"

using Booster = std::pair<char, Point>;

struct CellType {
  static constexpr int kEmpty = 0;
  static constexpr int kWrapped = 1;
  static constexpr int kObstacle = 2;
};

struct Game {
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

  // State of Game
  std::string command;
  int time = 0;

  // State of Map ======================================
  static const char NON_WRAPPED = '.';
  static const char WRAPPED = ' ';
  static const char WRAPPY = '@';
  static const char BOOSTER_MANIPULATOR = 'B';
  static const char BOOSTER_FAST_WHEEL = 'F';
  static const char BOOSTER_DRILL = 'L';
  static const char WALL = '#';
  // static const char UNKNOWN = 'X';

  // To display a map, use ostream::operator<<.
  // Y direction maybe wrong.
  Map2D map2d;
  std::vector<std::string> map;

  // State of Wrappy ===================================
  Point wrappy;
  std::vector<Point> manipulators;

  // Boosters
  int num_manipulators = 0;  // number of unused manipulators
  int fast_wheels = 0;  // Increase if take 'F'
  int drills = 0;  // Increase if take 'L'
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
