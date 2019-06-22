#include <memory>
#include <string>

#include "action.h"
#include "game.h"
#include "wrapper.h"
#include "solver_registry.h"

std::string simpleSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  for (auto& wrapper : game->wrappers) {
    wrapper->move(Action::UP);
  }
  game->tick();
  displayAndWait(param, game);
  if (!iter_callback(game)) return game->getCommand();
  return game->getCommand();
}

REGISTER_SOLVER("simple", simpleSolver);
