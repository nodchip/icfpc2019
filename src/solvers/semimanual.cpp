#include <iostream>
#include <cctype>
#include <queue>
#include <map>

#include "map_parse.h"
#include "solver_helper.h"
#include "solver_registry.h"

struct CommandBase {
  virtual ~CommandBase() {}
};
struct Spawn : public CommandBase {
  Spawn(Point p) : pos(p) {}
  Point pos;
};
struct PickC : public CommandBase {
  PickC(Point p) : pos(p) {}
  Point pos;
};
struct SetCoverCenter : public CommandBase {
  SetCoverCenter(Point p) : pos(p) {}
  Point pos;
};
struct Plan {
  Game* game;
  std::vector<std::queue<CommandBase*>> commands;
  std::map<int, Point> cover_centers;
  bool debug = false;

  Plan(Game* g) : game(g) {}
  void operator()(int i, CommandBase* cmd) {
    if (commands.size() <= i) {
      assert (commands.size() == i);
      commands.resize(i + 1);
    }
    commands[i].push(cmd);
  }

  void hasBoughtC(int c) {
    assert (game->num_boosters[BoosterType::CLONING] >= c);
  }

  void execute(SolverParam param, SolverIterCallback iter_callback) {
    std::vector<CommandBase*> processing_command(1, nullptr);
    std::vector<int> terminatable(1, 0);

    while (!game->isEnd()) {
      if (debug) std::cout << "===== " << game->time << std::endl;
      if (debug) std::cout << *game << std::endl;

      bool all_terminatable = true;
      for (int i = 0; i < terminatable.size(); ++i) {
        if (!terminatable[i]) {
          all_terminatable = false;
        }
      }
      if (all_terminatable) {
        if (debug) std::cout << "terminate plan" << std::endl;
        break;
      }

      for (int i = 0; i < game->wrappers.size(); ++i) {
        Wrapper* w = game->wrappers[i].get();
        assert (i < processing_command.size());
        if (debug) std::cerr << "t=" << game->time << " i=" << i << " pos=" << w->pos << std::endl;

        // continue or get new command.
        if (!processing_command[i]) {
          if (commands[i].empty()) {
            w->nop();
            continue;
          } else {
            if (debug) std::cerr << "pop next command" << std::endl;
            processing_command[i] = commands[i].front(); commands[i].pop();
          }
        }
        assert (processing_command[i]);
        CommandBase* cmd = processing_command[i];

        // continue or new command
        Point next {-1, -1};
        if (Spawn* c = dynamic_cast<Spawn*>(cmd)) {
          if (debug) std::cerr << "t=" << game->time << " i=" << i << " pos=" << w->pos << ": spawn X " << c->pos << std::endl;
          if (w->pos == c->pos) {
            if (game->num_boosters[BoosterType::CLONING] > 0) {
              // reached, spawn.
              w->cloneWrapper();
              processing_command.push_back(nullptr); // new.
              terminatable.push_back(0); // new.
              processing_command[i] = nullptr; // finished.
            } else {
              // wait.
              w->nop();
            }
          } else {
            // not reached, move.
            std::vector<Trajectory> trajs = map_parse::findTrajectory(*game, w->pos, c->pos, DISTANCE_INF);
            assert (!trajs.empty());
            w->move(Direction2Char(trajs[0].last_move));
          }
        } else if (PickC* c = dynamic_cast<PickC*>(cmd)) {
          if (debug) std::cerr << "t=" << game->time << " i=" << i << " pos=" << w->pos << ": pick C " << c->pos << std::endl;
          if (w->pos == c->pos) {
            // reached.
            w->nop(); // MUDA!
            processing_command[i] = nullptr; // finished.
          } else {
            // not reached, move.
            std::vector<Trajectory> trajs = map_parse::findTrajectory(*game, w->pos, c->pos, DISTANCE_INF);
            assert (!trajs.empty());
            w->move(Direction2Char(trajs[0].last_move));
          }
        } else if (SetCoverCenter* c = dynamic_cast<SetCoverCenter*>(cmd)) {
          if (debug) std::cerr << "t=" << game->time << " i=" << i << " pos=" << w->pos << ": set cover " << c->pos << std::endl;
          auto targets = findNearestPoints(enumerateCellsByMask(game->map2d, CellType::kObstacleBit, 0), c->pos);
          assert (!targets.empty());
          auto target = targets[0];
          if (w->pos == target) {
            // reached.
            w->nop(); // MUDA!
            processing_command[i] = nullptr; // finished.
            terminatable[i] = true;
          } else {
            // not reached, approach.
            std::vector<Trajectory> trajs = map_parse::findTrajectory(*game, w->pos, target, DISTANCE_INF);
            assert (!trajs.empty());
            w->move(Direction2Char(trajs[0].last_move));
          }
        } else {
          assert(false);
        }
      }
      game->tick();
      displayAndWait(param, game);
      if (iter_callback && !iter_callback(game)) return;
    }
  }
};

using ManualFunction = std::function<Plan(Game*)>;
std::map<int, ManualFunction> manual_specs = {
  {2, [](Game* g) -> Plan {
    Plan plan(g);
    plan.hasBoughtC(1);
    plan(0, new Spawn{{9, 19}});
    plan(0, new SetCoverCenter{{0, 0}});
    plan(1, new SetCoverCenter{{30, 30}});
    return plan;
  }},
  {242, [](Game* g) -> Plan {
    // X(135,149);X(123,39);X(132,57);X(143,69);C(122,113);C(152,171);C(80,116);
    Plan plan(g);
    plan(0, new PickC{{80, 116}});
    plan(0, new Spawn{{135, 149}});
    plan(0, new PickC{{152, 171}});
    plan(1, new Spawn{{132, 57}});
    plan(1, new PickC{{122, 113}});
    plan(2, new Spawn{{123, 39}});

    plan(0, new SetCoverCenter{{200, 200}});
    plan(1, new SetCoverCenter{{0, 200}});
    plan(2, new SetCoverCenter{{200, 0}});
    plan(3, new SetCoverCenter{{0, 0}});
    return plan;
  }},
};

std::string semimanualSolver(SolverParam param, Game* game, SolverIterCallback iter_callback) {
  std::cerr << "Problem #" << game->problem_no << std::endl;

  // if exists, execute semimanual plan
  if (manual_specs.find(game->problem_no) != manual_specs.end()) {
    std::cerr << "found semimanual plan" << std::endl;
    Plan plan = manual_specs[game->problem_no](game);
    plan.execute(param, iter_callback);
  }

  // continue (or start)
  // do not care about further cloning.
  ConnectedComponentAssignmentForParanoid cc_assignment(game,
    10 /* distance_threshold */,
    100 /* small_region_bonus */);
  
  std::vector<ManipulatorExtender> extenders;
  for (auto& w : game->wrappers) {
    extenders.emplace_back(game, w.get());
  }

  while (!game->isEnd()) {
    cc_assignment.delayUpdate();

    for (int i = 0; i < game->wrappers.size(); ++i) {
      auto& w = game->wrappers[i];
      auto pos = w->pos;

      // manipulator
      if (game->num_boosters[BoosterType::MANIPULATOR]) {
        extenders[i].extend();
        continue;
      }

      // paranoid
      std::vector<Trajectory> trajs;
      cc_assignment.update();
      if (cc_assignment.hasDisjointComponents() && cc_assignment.isComponentAssignedToWrapper(w->index)) {
        Trajectory t;
        if (pointToDirection(t.last_move, cc_assignment.getSuggestedMotionOfWrapper(w->index))) {
          trajs = { t };
        }
      }

      // BFS
      if (trajs.empty()) {
        trajs = map_parse::findNearestUnwrapped(*game, pos, DISTANCE_INF);
      }

      if (trajs.empty()) {
        w->nop();
      } else {
        w->move(Direction2Char(trajs[0].last_move));
      }
    }

    game->tick();
    displayAndWait(param, game);
    if (iter_callback && !iter_callback(game)) return game->getCommand();
  }

  std::cout << *game << std::endl;
  return game->getCommand();
}

REGISTER_SOLVER("semimanual", semimanualSolver);
