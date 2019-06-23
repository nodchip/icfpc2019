#include <iostream>
#include <cstdio>
#include <cstdint>
#include <queue>
#include <string>
#include <thread>
#include <chrono>
#include <unordered_map>

#include "booster.h"
#include "game.h"
#include "solver_registry.h"
#include "wrapper.h"

std::string sampoSolver(SolverParam, Game*, SolverIterCallback);
REGISTER_SOLVER("sampo", sampoSolver);

namespace std {

template<>
class hash<Point> {
public:
  size_t operator()(const Point& p) const { return (p.x << 16) + p.y; }
};

}  // namespace std

namespace {

// Directions. Upper 4 bits is 'to', lower 4 bits is 'from'.
using Direction = uint8_t;

struct State {
  Point target = {-1, -1};
  std::unordered_map<Point, Direction> direction_map;
};

void decideWrapperAction(Wrapper*, State&);

}  // namespace

std::string sampoSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  std::vector<State> states(1);

  while (!game->isEnd()) {
    if (game->wrappers.size() != states.size()) {
      states.resize(game->wrappers.size());
    }

    for (auto& wrapper : game->wrappers) {
      decideWrapperAction(wrapper.get(), states[wrapper->index]);
    }

    game->tick();
    displayAndWait(param, game);
    if (iter_callback && !iter_callback(game))
      return game->getCommand();
  }
  return game->getCommand();
}

namespace {

namespace Target {
static constexpr int kUnwrapped = 0;
static constexpr int kManipulator = 1;
static constexpr int kFastWheel = 2;
static constexpr int N = 3;
// static constexpr int kDrill = 3;
// static constexpr int kCloning = 4;
// static constexpr int kBeacon = 5;
// static constexpr int kSpawn = 6;
};

bool useBoosters(Wrapper* w) {
  Game* game = w->game;
  if (game->num_boosters[BoosterType::MANIPULATOR] > 0) {
    int dx = 2;
    while (!w->canAddManipulator(Point{dx, 0}))
      ++dx;
    w->addManipulator(Point{dx, 0});
    return true;
  }
  if (game->num_boosters[BoosterType::FAST_WHEEL] > 0 &&
      w->time_fast_wheels == 0) {
    w->useBooster('F');
    return true;
  }
  // TODO: Use drill if it is useful
#if 0
  if (game->num_boosters[BoosterType::DRILL] > 0 &&
      w->time_drill == 0) {
    w->useBooster('L');
    return true;
  }
#endif
  // TODO: Use clone if it is useful (everyone moves same)
#if 0
  if (game->num_boosters[BoosterType::CLONING] > 0 &&
      (game->map2d(w->pos) & CellType::kSpawnPointBit)) {
    w->useBooster('C');
    return true;
  }
#endif

  return false;
}

constexpr Direction kUp    = 1;
constexpr Direction kDown  = 2;
constexpr Direction kLeft  = 3;
constexpr Direction kRight = 4;
constexpr Direction kFromMask = 0xf;
constexpr Direction kToMask = 0xf0;

inline Direction setFrom(const Direction dir, const Direction from) {
  return (dir & kToMask) | from;
}
inline Direction getFrom(const Direction& dir) {
  return dir & kFromMask;
}
inline Direction setTo(const Direction dir, const Direction to) {
  return (dir & kFromMask) | (to << 4);
}
inline Direction getTo(const Direction dir) {
  return (dir >> 4) & kFromMask;
}

inline bool isUnwrapped(int c) {
  return (c & (CellType::kObstacleBit | CellType::kWrappedBit)) == 0;
}

void decideWrapperAction(Wrapper* w, State& state) {
  auto& direction_map = state.direction_map;

  // If already the target is set, go there.
  if (Direction to = getTo(direction_map[w->pos])) {
    switch (to) {
    case kUp:    w->move('W'); break;
    case kDown:  w->move('S'); break;
    case kLeft:  w->move('A'); break;
    case kRight: w->move('D'); break;
    }
    return;
  }

  if (useBoosters(w))
    return;

  direction_map.clear();
  Map2D& map = w->game->map2d;

  // NOTE: This simplifies some searches. Do not forget to put it back.
  const int current = map(w->pos);
  map(w->pos) = CellType::kWrappedBit;

  int target_bits = 0;
  for (int x = 0; x < map.W; ++x) {
    for (int y = 0; y < map.H; ++y) {
      target_bits |= map(x, y);
    }
  }
  target_bits &= (CellType::kWrappedBit | CellType::kBoosterManipulatorBit | CellType::kBoosterFastWheelBit);

  std::unordered_map<Point, int> cost_map;
  std::queue<Point> q;
  cost_map[w->pos] = 0;
  q.push(w->pos);
  std::vector<Point> targets(Target::N, Point{-1, -1});
  while (!q.empty()) {
    Point from = q.front();
    q.pop();

    if ((target_bits & CellType::kWrappedBit) && isUnwrapped(map(from))) {
      targets[Target::kUnwrapped] = from;
      target_bits &= ~CellType::kWrappedBit;
    }
    if (target_bits & CellType::kBoosterManipulatorBit & map(from)) {
      targets[Target::kManipulator] = from;
      target_bits &= ~CellType::kBoosterManipulatorBit;
    } else if (target_bits & CellType::kBoosterFastWheelBit & map(from)) {
      targets[Target::kFastWheel] = from;
      target_bits &= ~CellType::kBoosterFastWheelBit;
    }

    if (!target_bits)
      break;

    int cost = cost_map[from] + 1;
    static const struct {
      Point dp;
      Direction from;
    } dps[] = {
      {{0, -1}, kDown},
      {{0, 1}, kUp},
      {{1, 0}, kRight},
      {{-1, 0}, kLeft},
    };
    for (auto& dp : dps) {
      Point p = from - dp.dp;
      if (!map.isInside(p))
        continue;
      if ((map(p) & CellType::kObstacleBit) && w->time_drill == 0)
        continue;
      if (cost_map.count(p) == 0 || cost_map[p] > cost) {
        cost_map[p] = cost;
        direction_map[p] = setFrom(direction_map[p], dp.from);
        q.push(p);
      }
    }
  }

  Point& goal = state.target;
  if (cost_map.count(targets[Target::kManipulator]) && cost_map[targets[Target::kManipulator]] < 10) {
    goal = targets[Target::kManipulator];
  } else if (cost_map.count(targets[Target::kFastWheel]) && cost_map[targets[Target::kFastWheel]] < 5) {
    goal = targets[Target::kFastWheel];
  } else {
    goal = targets[Target::kUnwrapped];
  }

  for (Point p = goal; p != w->pos;) {
    switch (getFrom(direction_map[p])) {
    case kUp:    ++p.y; direction_map[p] = setTo(direction_map[p], kDown); break;
    case kDown:  --p.y; direction_map[p] = setTo(direction_map[p], kUp); break;
    case kLeft:  --p.x; direction_map[p] = setTo(direction_map[p], kRight); break;
    case kRight: ++p.x; direction_map[p] = setTo(direction_map[p], kLeft); break;
    default:
      assert(false);
    }
  }

  switch (getTo(direction_map[w->pos])) {
  case kUp:    w->move('W'); break;
  case kDown:  w->move('S'); break;
  case kLeft:  w->move('A'); break;
  case kRight: w->move('D'); break;
  }

  map(w->pos) = current;
}

}  // namespace
