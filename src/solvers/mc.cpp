// mc.cpp : [M]anipulator - [C]loning
#include <iostream>
#include <cctype>

#include "map_parse.h"
#include "solver_registry.h"
#include "solver_helper.h"

std::string mcSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  std::vector<Point> Bs = enumerateCellsByMask(game->map2d, CellType::kBoosterManipulatorBit, CellType::kBoosterManipulatorBit);
  std::vector<Point> Cs = enumerateCellsByMask(game->map2d, CellType::kBoosterCloningBit, CellType::kBoosterCloningBit);
  std::vector<Point> Xs = enumerateCellsByMask(game->map2d, CellType::kSpawnPointBit, CellType::kSpawnPointBit);
  std::cout << "B: " << Bs.size() << ", C: " << Cs.size() << ", X: " << Xs.size() << std::endl;

  std::map<Wrapper*, ManipulatorExtender*> manipulator_extender; // とりあえずリークは無視
  Wrapper* w = game->wrappers[0].get();
  manipulator_extender[w] = new ManipulatorExtender(game, w);
  bool collect_b = true;
  bool collect_c = Cs.size() > 0 && Xs.size() > 0;
  bool spawn_x = Cs.size() > 0 && Xs.size() > 0;

  return functorSolver(param, game, iter_callback, [&](Wrapper* w) -> Wrapper* {
    if (game->num_boosters[BoosterType::MANIPULATOR] > 0) { // Bがあれば使う
      manipulator_extender[w]->extend();
      return nullptr;
    }

    if (collect_b) { // 最も近いBを探して向かう
      std::vector<Trajectory> trajs = map_parse::findNearestByBit(*game, w->pos, DISTANCE_INF, CellType::kBoosterManipulatorBit); 
      if (trajs.size() > 0) {
        for(auto t : trajs) {
          const char c = Direction2Char(t.last_move);
          w->move(c);
          return nullptr;
        }
      }
      collect_b = false;
    }

    if (collect_c) { // Bを取り尽くしたのでCを集める
      std::vector<Trajectory> trajs = map_parse::findNearestByBit(*game, w->pos, DISTANCE_INF, CellType::kBoosterCloningBit); 
      if (trajs.size() > 0) {
        for(auto t : trajs) {
          const char c = Direction2Char(t.last_move);
          w->move(c);
          return nullptr;
        }
      }
      collect_c = false;
    }

    if (game->num_boosters[BoosterType::CLONING] == 0) { // CがなくなったらもうXはいらない
      spawn_x = false;
    } else if ((game->map2d(w->pos) & CellType::kSpawnPointBit) != 0) { // X上でCできればする
      Wrapper* cloned = w->cloneWrapper();
      manipulator_extender[cloned] = new ManipulatorExtender(game, cloned);
      return cloned;
    }

    if (w->index == 0 && spawn_x) { // Cを取り尽くしたのでXする(primary botのみ)
      std::vector<Trajectory> trajs = map_parse::findNearestByBit(*game, w->pos, DISTANCE_INF, CellType::kSpawnPointBit); 
      if (trajs.size() > 0) {
        for(auto t : trajs) {
          const char c = Direction2Char(t.last_move);
          w->move(c);
          return nullptr;
        }
      }
    }

    // B, C, X を終えたので好きに動く
    const std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(*game, w->pos, DISTANCE_INF);

    int count = game->countUnWrapped();
    if (trajs.size() == 0)
      return nullptr;

    for(auto t : trajs){
      const char c = Direction2Char(t.last_move);
      w->move(c);
      return nullptr;
    }

    return nullptr;
  });
}

REGISTER_SOLVER("mc", mcSolver);

