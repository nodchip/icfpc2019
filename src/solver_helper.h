#pragma once

#include <vector>
#include <memory>
#include "wrapper.h"
#include "game.h"
#include "solver_registry.h"

// override this.
struct WrapperEngineBase {
  using Ptr = std::shared_ptr<WrapperEngineBase>;

  WrapperEngineBase() = default;
  WrapperEngineBase(Game *game, Wrapper* wrapper)
   : m_game(game), m_wrapper(wrapper) {
  }
  virtual ~WrapperEngineBase() {}
  virtual Ptr create(Game* game, Wrapper *wrapper) = 0;
  virtual Wrapper* action(const std::vector<WrapperEngineBase*>& other_engines) = 0;

protected:
  Game *m_game = nullptr;
  Wrapper *m_wrapper = nullptr;
};

std::string wrapperEngineSolver(SolverParam param, Game* game, SolverIterCallback iter_callback, WrapperEngineBase::Ptr prototype);
