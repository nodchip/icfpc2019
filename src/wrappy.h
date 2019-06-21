#pragma once

#include <string>

#include "base.h"
#include "map.h"

struct Wrappy {
  explicit Wrappy(Map& map);
  void move(char);  // input: WSAD
  void nop();  // input: Z
  void turn(char);  // input: EQ
  void addManipulate(int, int);  // input: x,y
  void useBooster(char);  // input: FL

  Point point;
  std::vector<Point> manipulators;
  int fast_wheels = 0; // Increase if take 'F'
  int drills = 0;  // Increase if take 'L'
  // remained time of 'F'. While this is >0, speed becomes 2.
  int time_fast_wheels = 0;
  // remained time of 'L'. While this is >0, wrappy can go through obstacles.
  int time_drill = 0;
  Map& map;
};
