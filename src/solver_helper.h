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
std::string functorSolver(SolverParam param, Game* game, SolverIterCallback iter_callback, std::function<Wrapper*(Wrapper*)> func);

struct ManipulatorExtender {
  //   :
  //   2
  //   0
  //   x
  // @ x
  //   x
  //   1
  //   3
  //   :
  ManipulatorExtender(Game* game_, Wrapper* wrapper_) : game(game_), wrapper(wrapper_) {}

  // this will always emit an action.
  void extend();
  int count() const { return num_attached_manipulators; }
private:
  Game *game;
  Wrapper *wrapper;
  int num_attached_manipulators = 0;
};

std::vector<std::vector<Point>> disjointConnectedComponentsByMask(const Map2D& map, int mask, int bits);

// since it is crutial to reach to C as soon as possible,
// consider whether picking and using of F might be useful.
// start -> F(use immediately) -> C
// NOTE: incomplete. it may be difficult to control wrapper through narrow road...
struct FindFCRouteResult {
  Point F_pos;
  Point C_pos;
  int time_cost = 0; // approx.
};
std::unique_ptr<FindFCRouteResult> findGoodFCRoute(const Map2D& map, Point start);