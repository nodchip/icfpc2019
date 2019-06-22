#include <memory>
#include <string>

#include "action.h"
#include "game.h"
#include "wrapper.h"
#include "solver_registry.h"

std::string simpleSolver(std::shared_ptr<Game> game) {
  for (auto wrapper : game->wrappers) {
    wrapper->move(Action::UP);
  }
  game->tick();
  return game->getCommand();
}

REGISTER_SOLVER("simple", simpleSolver);
