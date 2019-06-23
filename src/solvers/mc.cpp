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
  bool collect_c = false && Cs.size() > 0 && Xs.size() > 0;
  bool spawn_x = false && Cs.size() > 0 && Xs.size() > 0;

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

    if (trajs.size() > 0) {
      // k stepの間に塗れる数が増えるならturnする
      const int k = 8;
      int painted[4] = {0};
      int best_iturn = 0;
      for (int iturn = 0; iturn < 4; ++iturn) {
        int n_undo = 0;
        // iturn回の回転
        for (; n_undo < iturn; ++n_undo) {
          w->turn(Action::CCW);
          painted[iturn] += w->getLastNumWrapped();
        }
        // 残りを移動に使う
        for (; n_undo < std::min<int>(k, trajs.size()); ++n_undo) {
          const char c = Direction2Char(trajs[n_undo].last_move);
          w->move(c);
          painted[iturn] += w->getLastNumWrapped();
        }
        while (n_undo--) {
          w->undoAction();
        }
        if (painted[best_iturn] < painted[iturn]) {
          best_iturn = iturn;
        }
      }
      // 最も良いものを選ぶ
      // iturn回の回転
      for (int i = 0; i < best_iturn; ++i) {
        w->turn(Action::CCW);
        game->tick();
      }
    }

    w->nop();
    return nullptr;
  });
}

REGISTER_SOLVER("mc", mcSolver);

