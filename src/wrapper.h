#pragma once

#include <ostream>
#include <string>
#include <vector>
#include <memory>

#include "action.h"
#include "base.h"
#include "map2d.h"
#include "booster.h"

struct Game;

struct Wrapper {
  Wrapper(Game* game_, Point pos_, int wrapper_spawn_index_);

  // Control Wrappy
  bool move(char);  // input: WSAD
  void nop();  // input: Z
  void turn(char);  // input: EQ
  bool addManipulator(const Point&);  // input: x,y relative to wrapper position.
  bool teleport(const Point&);  // input: x,y
  bool useBooster(char);  // input: FLR
  Wrapper* cloneWrapper(); 

  Action getScaffoldAction();
  std::string getCommand() const; // command for this wrapper.
  bool undoAction(); // if no actions are stacked, fail and return false.

  bool isMoveable(char);
  bool canAddManipulator(const Point&);
  int getLastNumWrapped() {
    return actions.empty() ? 0 : actions.back().absolute_new_wrapped_positions.size();
  }

  Game* game;
  Point pos;
  int index;
  Direction direction;
  std::vector<Action> actions;
  std::vector<Point> manipulators;

  // remained time of 'F'. While this is >0, speed becomes 2.
  int time_fast_wheels = 0;
  // remained time of 'L'. While this is >0, wrappy can go through obstacles.
  int time_drill = 0;
  // Gather wrappre stat information
  WrapperStat wrapper_stat;

private:
  void pick(Action& a);
  void moveAndPaint(Point p, Action& a);
  void doAction(Action a);
};
