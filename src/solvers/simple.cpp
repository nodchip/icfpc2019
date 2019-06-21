#include "../solver_registry.h"

std::string simpleSolver(Game game) {
    game.move(Game::UP);
    return game.command;
}

REGISTER_SOLVER("simple", simpleSolver);