#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "base.h"
#include "map2d.h"
#include "wrapper.h"

struct Game {
  Game() = default;
  Game(const std::string& desc); // initialize using a task description string from *.desc file.
  Game(const std::vector<std::string>& map); // initialize by a raster *.map file.

  bool tick(); // move time frame. make sure you provided commands for each wrapper.

  bool undo(); // undo previous frame. if no actions are stacked, fail and return false.
  std::string getCommand() const; // extended solution command.

  int nextWrapperIndex() const { return wrappers.size(); }
  std::vector<Point> getWrapperPositions() const;

  void paint(const Wrapper& w, Action& a); // helper func used by Wrapper

  // State of Game
  int time = 0;

  // shared & updated map
  Map2D map2d;

  // State of Wrappy ===================================
  std::vector<std::shared_ptr<Wrapper>> wrappers;

  // Boosters (shared among wrappers)
  int num_manipulators = 0;  // number of unused manipulators
  int fast_wheels = 0;  // Increase if take 'F'
  int drills = 0;  // Increase if take 'L'
  int teleports = 0;  // Increase if take 'R'
  int clonings = 0; // Increase if take 'C'

private:
  Action getScaffoldAction();
};

// Outputs game status
std::ostream& operator<<(std::ostream&, const Game&);
