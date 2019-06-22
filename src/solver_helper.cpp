#include "solver_helper.h"

std::string wrapperEngineSolver(SolverParam param, Game* game, SolverIterCallback iter_callback, WrapperEngineBase::Ptr prototype) {
  std::vector<WrapperEngineBase::Ptr> engines;
  engines.emplace_back(prototype->create(game, game->wrappers[0].get()));

  while (game->countUnWrapped() != 0) {
    std::vector<int> cloned_ids;
    for (auto& e : engines) {
      std::vector<WrapperEngineBase*> other_engines;
      for (auto& o : engines) {
        if (e.get() != o.get()) {
            other_engines.push_back(o.get());
        }
      }
      if (Wrapper* e_cloned = e->action(other_engines)) {
        cloned_ids.emplace_back(e_cloned->index);
      }
    }

    game->tick();
    displayAndWait(param, game);
    if (iter_callback && !iter_callback(game)) return game->getCommand();

    for (auto id : cloned_ids) {
      engines.emplace_back(prototype->create(game, game->wrappers[id].get()));
    }
  }
  return game->getCommand();
}
