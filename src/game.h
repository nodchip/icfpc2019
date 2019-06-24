#pragma once

#include <ostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>

#include "base.h"
#include "map2d.h"
#include "wrapper.h"
#include "booster.h"

struct Buy {
  Buy();
  Buy(const std::string& buy_desc);
  static Buy fromFile(const std::string& file_path);

  bool empty() const;
  std::string toString() const;
  std::array<int, BoosterType::N> boosters;
};

struct Game {
  Game(const std::string& desc); // initialize using a task description string from *.desc file.
  Game(const std::vector<std::string>& map); // initialize by a raster *.map file.
  Game(const Game& another);
  Game& operator=(const Game& another);

  void buyBoosters(const Buy& buy);

  void clearDebugKeyValues() { debug_keyvalues.clear(); }

  template <typename T>
  void addDebugKeyValue(std::string key, T val) {
    std::ostringstream oss;
    oss << val;
    debug_keyvalues.emplace_back(key, oss.str());
  }

  // move time frame. make sure you provided commands for each wrapper.
  bool tick();

  // undo previous frame. if no actions are stacked, fail and return false.
  bool undo();

  // Returns True if no unwrapped cells are remained.
  bool isEnd() const;

  int countUnwrapped() const { return map2d.num_unwrapped; }

  std::string getCommand() const; // extended solution command.

  int nextWrapperIndex() const { return wrappers.size() + next_wrappers.size(); }
  void addClonedWrapperForNextFrame(std::unique_ptr<Wrapper> wrapper); // this wrapper will be available after tick()
  std::vector<Point> getWrapperPositions() const;

  // according to the rules, a tick consists of:
  // for i in [0..N]
  //   1. wrapper[i] picks anything in the cell
  //   2. wrapper[i] moves (e.g. use any boosters collected)
  void pick(const Point& p, Action* a_optional); // helper func used by Wrapper
  void paint(const Wrapper& w, Action* a_optional); // helper func used by Wrapper

  // State of Game
  int time = 0;

  // shared & updated map
  Map2D map2d;

  // State of Wrappy ===================================
  std::vector<std::unique_ptr<Wrapper>> wrappers;

  // Unused boosters (shared among wrappers)
  std::array<int, BoosterType::N> num_boosters;

private:
  Game();
  std::vector<std::unique_ptr<Wrapper>> next_wrappers;
  std::vector<std::pair<std::string, std::string>> debug_keyvalues;
  friend std::ostream& operator<<(std::ostream&, const Game&);
};

bool checkCommandString(std::string);

std::vector<std::string> dumpMapStringWithManipulators(const Map2D& map2d, const std::vector<std::unique_ptr<Wrapper>>& wrappers);

// Outputs game status
std::ostream& operator<<(std::ostream&, const Game&);
