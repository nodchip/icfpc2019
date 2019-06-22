#include "../solver_registry.h"

std::string simpleSolver(std::shared_ptr<Game> game) {
    game->wrappers[0]->move(Wrapper::UP);
    game->tick();
    return game->getCommand();
}

REGISTER_SOLVER("simple", simpleSolver);