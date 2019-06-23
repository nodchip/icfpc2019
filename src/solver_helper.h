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

// =======================
// tick()ごとに更新し、非連結領域がある場合はそれぞれを個別にwrapperにアサインする
// 通常一手前の行動でごく近傍に少領域/大領域境界を生成する
// 小さな塗り残しをさけつつ、また他のwrapperのほうが近くにい少領域を避けるため、領域重心に近いものにアサインされやすくする
namespace detail {
using weight = int;
using matrix = std::vector<std::vector<weight>>;
// http://www.prefield.com/algorithm/math/hungarian.html + mod.
weight hungarian(const matrix &a, std::vector<int>& x, std::vector<int>& y);
}
struct ConnectedComponentAssignmentForParanoid {
  static const int UNASSIGNED = -1;

  ConnectedComponentAssignmentForParanoid(Game* game_, int distance_threshold_);

  bool hasDisjointComponents() const;
  bool isComponentAssignedToWrapper(int i) const;
  Point getTargetOfWrapper(int i) const;
  Point getSuggestedMotionOfWrapper(int i) const;

  void delayUpdate();
  bool update(); // true: delay-updated, false: not updated.

private:
  struct Component {
    std::vector<Point> points;
    Point center; // approx. centroid of points.
    Point suggested_motion; // one of neighbor-4
    Point target;
  };

  Game* game = nullptr;
  int distance_threshold = 100;
  bool delay_update_flag = false;
  std::vector<Component> components;
  std::vector<int> wrapper_to_component;

  static Point approximateCenter(const std::vector<Point>& points);
};
