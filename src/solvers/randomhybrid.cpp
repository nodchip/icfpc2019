#include <iostream>
#include <sstream>
#include <cctype>
#include <cstdlib>

#include "map_parse.h"
#include "solver_registry.h"

template <typename T>
T getEnv(std::string name, T default_value) {
  if (const char *env = std::getenv(name.c_str())) {
    std::istringstream iss(env);
    T result {};
    iss >> result;
    return result;
  }
  return default_value;
}


std::string randomHybridSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  const int DIVERGE_STEP = getEnv<int>("DIVERGE_STEP", 30);
  const int SEARCH_STEP = getEnv<int>("SEARCH_STEP", 50);

  std::vector<std::string> solver_names = {
    "bfs3_plus_dircheck",
    "bfs3_plus_wipe",
    "bfs4",
  };
  std::vector<SolverFunction> solvers;
  for (auto name : solver_names) {
    for (auto it : SolverRegistry<SolverFunction>::getRegistry()) {
      if (it.first == name) {
        solvers.push_back(it.second.function);
      }
    }
  }
  assert (!solvers.empty());
  int selected = 0;

  bool terminate = false;

  while (!game->isEnd()) {
    { // move for a while
      int i = 0;
      SolverIterCallback stop_at_step = [&](Game* g) {
        if (iter_callback && !iter_callback(g)) {
          terminate = true;
          return false;
        }
        return (++i < DIVERGE_STEP);
      };
      solvers[selected](param, game, stop_at_step);
    }
    if (terminate || game->isEnd()) {
      break;
    }

    std::cout << "try solvers.. " << game->time << std::endl;
    // try solvers.
    int best_selected = selected;
    int best_total = 0;
    for (int j = 0; j < solvers.size(); ++j) {
      int i = 0;
      int total = 0;
      const int time_before = game->time;
      SolverIterCallback stop_at_step = [&](Game* g) {
        for (auto& w : game->wrappers) {
          total += w->actions.back().wrappedCount();
        }
        return (++i < SEARCH_STEP);
      };
      // move for a while
      solvers[j](param, game, stop_at_step);
      // revert
      while (game->time > time_before) {
        game->undo();
      }
      if (best_total < total) {
        best_selected = j;
        best_total = total;
      }
      //std::cout << "  [" << solver_names[j] << "] = " << total << std::endl;
    }

    if (best_selected != selected) {
      std::cout << "switched to: " << solver_names[best_selected] << std::endl;
      selected = best_selected;
    }
  }
  return game->getCommand();
}

REGISTER_SOLVER("random_hybrid", randomHybridSolver);
