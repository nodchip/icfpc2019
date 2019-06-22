#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"
#include "solver_helper.h"

namespace {
struct WrapperEngine : public WrapperEngineBase {
  WrapperEngine() = default;
  WrapperEngine(Game *game, Wrapper *wrapper)
    : WrapperEngineBase(game, wrapper)
    , m_num_manipulators(0) {};

  virtual Ptr create(Game* game, Wrapper *wrapper) {
    return std::make_shared<WrapperEngine>(game, wrapper);
  }

  virtual Wrapper* action(const std::vector<WrapperEngineBase*>&) {
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*m_game, m_wrapper->pos, DISTANCE_INF);
    if (trajs.size() == 0) {
      m_wrapper->nop();
      return nullptr;
    }
    const char c = Direction2Char(trajs[0].last_move);
    m_wrapper->move(c);
    return nullptr;
  }
  int m_num_manipulators;
};
}

std::string solverHelperTestSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  return wrapperEngineSolver(param, game, iter_callback, std::make_shared<WrapperEngine>());
}

REGISTER_SOLVER("solver_helper_test", solverHelperTestSolver);

