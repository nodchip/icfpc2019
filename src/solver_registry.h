#pragma once
#include <string>
#include <functional>
#include <map>
#include <cassert>
#include "game.h"

struct SolverParam {
  int wait_ms = 0;
};

void displayAndWait(SolverParam param, std::shared_ptr<Game> game);

using SolverFunction = std::function<std::string(SolverParam, std::shared_ptr<Game>)>;

#define REGISTER_SOLVER(name, func) \
  static SolverRegistry _register_solver_##__COUNT__ = {name, {__FILE__, func}}

struct SolverRegistry {
  struct SolverEntry {
    std::string file_name;
    SolverFunction function;
  };
  static std::map<std::string, SolverEntry>& getRegistry() {
    static std::map<std::string, SolverEntry> s_solver_registry;
    return s_solver_registry;
  }
  static SolverFunction getSolver(std::string name) {
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