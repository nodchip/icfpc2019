#pragma once
#include <string>
#include <functional>
#include <map>
#include <cassert>
#include "game.h"
#include "puzzle.h"

struct SolverParam {
  int wait_ms = 0;
};
struct PuzzleSolverParam {
  int wait_ms = 0;
};

void displayAndWait(SolverParam param, Game* game);

using SolverIterCallback = std::function<bool(Game*)>; // return false to terminate.
using SolverFunction = std::function<std::string(SolverParam, Game*, SolverIterCallback)>;
using PuzzleSolverFunction = std::function<Polygon(PuzzleSolverParam, Puzzle)>;

#define CONCAT_SUB(a, b) a##b
#define CONCAT(a, b) CONCAT_SUB(a, b)
#define REGISTER_SOLVER(name, func) \
  static SolverRegistry<SolverFunction> CONCAT(_register_solver_, __LINE__) = {name, {__FILE__, func}}
#define REGISTER_PUZZLE_SOLVER(name, func) \
  static SolverRegistry<PuzzleSolverFunction> CONCAT(_register_solver_, __LINE__) = {name, {__FILE__, func}}

template <typename Func>
struct SolverRegistry {
  struct SolverEntry {
    std::string file_name;
    Func function;
  };
  static std::map<std::string, SolverEntry>& getRegistry() {
    static std::map<std::string, SolverEntry> s_solver_registry;
    return s_solver_registry;
  }
  static Func getSolver(std::string name) {
      auto reg = getRegistry();
      auto it = reg.find(name);
      assert (it != reg.end());
      return it->second.function;
  }

  SolverRegistry(std::string name, SolverEntry entry) {
    getRegistry()[name] = entry;
  }

  static void displaySolvers();
};
