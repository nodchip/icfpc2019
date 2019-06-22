#include "../solver_registry.h"

std::string simpleSolver(Game& game) {
    game.move(Game::UP);
    return game.getCommand();
}

REGISTER_SOLVER("simple", simpleSolver);