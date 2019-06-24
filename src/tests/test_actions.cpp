#include <gtest/gtest.h>

#include "action.h"
#include "booster.h"
#include "base.h"
#include "game.h"

TEST(ActionTest, SimpleMoves) {
  Game game("(0,0),(3,0),(3,5),(0,5)#(0,0)##B(0,1);;F(0,2);L(0,3)");
  // ... 4
  // L.. 3
  // F.. 2
  // B.. 1
  // @.. 0
  // 012

  Wrapper* wrapper = game.wrappers[0].get();
  EXPECT_TRUE((game.map2d(0, 1) & CellType::kWrappedBit) == 0);
  wrapper->move(Action::UP); game.tick();  // (0,1)
  EXPECT_TRUE((game.map2d(0, 2) & CellType::kWrappedBit) == 0);
  wrapper->turn(Action::CCW); game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::MANIPULATOR]);
  EXPECT_FALSE((game.map2d(0, 2) & CellType::kWrappedBit) == 0);
  wrapper->turn(Action::CW); game.tick();

  wrapper->move(Action::UP); game.tick();  // (0,2)
  EXPECT_EQ(0, game.num_boosters[BoosterType::FAST_WHEEL]);
  wrapper->move(Action::UP); game.tick();  // (0,3)
  EXPECT_EQ(1, game.num_boosters[BoosterType::FAST_WHEEL]);
  wrapper->move(Action::UP); game.tick();  // (0,4)
  EXPECT_EQ(1, game.num_boosters[BoosterType::DRILL]);
  EXPECT_EQ(Point(0, 4), wrapper->pos);

  wrapper->move(Action::RIGHT); game.tick();
  EXPECT_EQ(Point(1, 4), wrapper->pos);
  wrapper->move(Action::DOWN); game.tick();
  EXPECT_EQ(Point(1, 3), wrapper->pos);
  wrapper->move(Action::LEFT); game.tick();
  EXPECT_EQ(Point(0, 3), wrapper->pos);

  wrapper->addManipulator({-1, 0}); game.tick();
  EXPECT_EQ(0, game.num_boosters[BoosterType::MANIPULATOR]);
  wrapper->useBooster(Action::FAST); game.tick();
  EXPECT_EQ(0, game.num_boosters[BoosterType::FAST_WHEEL]);
}

TEST(ActionTest, Clone) {
  // example-01.desc + mod.
  Game game("(0,0),(10,0),(10,10),(0,10)#(0,0)#(4,2),(6,2),(6,7),(4,7);(5,8),(6,8),(6,9),(5,9)#C(0,1);X(0,2)");

  Wrapper* wrapper = game.wrappers[0].get();
  //
  wrapper->move(Action::UP);
  game.tick();
  //
  wrapper->move(Action::UP);
  game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::CLONING]);
  //
  Wrapper* wrapper_cloned = wrapper->cloneWrapper();
  EXPECT_EQ(1, game.wrappers.size()); // not added yet.
  game.tick();
  EXPECT_EQ(2, game.wrappers.size()); // added.
  EXPECT_EQ(wrapper_cloned, game.wrappers[1].get());
  //
  wrapper->move(Action::UP);
  wrapper_cloned->move(Action::RIGHT);
  game.tick();
  EXPECT_EQ(Point(0, 3), wrapper->pos);
  EXPECT_EQ(Point(1, 2), wrapper_cloned->pos);
}

TEST(ActionTest, Manipulator) {
  // example-01.desc + mod.
  Game game("(0,0),(10,0),(10,10),(0,10)#(0,0)#(4,2),(6,2),(6,7),(4,7);(5,8),(6,8),(6,9),(5,9)#B(0,1);B(0,2)");

  Wrapper* wrapper = game.wrappers[0].get();
  //
  wrapper->move(Action::UP);
  game.tick();
  //
  wrapper->move(Action::UP);
  game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::MANIPULATOR]);
  //
  wrapper->move(Action::UP);
  game.tick();
  EXPECT_EQ(2, game.num_boosters[BoosterType::MANIPULATOR]);
  //
  wrapper->turn(Action::CCW);
  game.tick();

  // . o o o .
  // o * * * o
  // . o @ o .
  // . . o . .
  EXPECT_FALSE(wrapper->canAddManipulator({-2, -1}));
  EXPECT_FALSE(wrapper->canAddManipulator({-1, -1}));
  EXPECT_TRUE (wrapper->canAddManipulator({ 0, -1}));
  EXPECT_FALSE(wrapper->canAddManipulator({ 1, -1}));
  EXPECT_FALSE(wrapper->canAddManipulator({ 2, -1}));
  
  EXPECT_FALSE(wrapper->canAddManipulator({-2, 0}));
  EXPECT_TRUE (wrapper->canAddManipulator({-1, 0}));
  EXPECT_FALSE(wrapper->canAddManipulator({ 0, 0}));
  EXPECT_TRUE (wrapper->canAddManipulator({ 1, 0}));
  EXPECT_FALSE(wrapper->canAddManipulator({ 2, 0}));

  EXPECT_TRUE (wrapper->canAddManipulator({-2, 1}));
  EXPECT_FALSE(wrapper->canAddManipulator({-1, 1}));
  EXPECT_FALSE(wrapper->canAddManipulator({ 0, 1}));
  EXPECT_FALSE(wrapper->canAddManipulator({ 1, 1}));
  EXPECT_TRUE (wrapper->canAddManipulator({ 2, 1}));

  EXPECT_FALSE(wrapper->canAddManipulator({-2, 2}));
  EXPECT_TRUE (wrapper->canAddManipulator({-1, 2}));
  EXPECT_TRUE (wrapper->canAddManipulator({ 0, 2}));
  EXPECT_TRUE (wrapper->canAddManipulator({ 1, 2}));
  EXPECT_FALSE(wrapper->canAddManipulator({ 2, 2}));

  // * * *  
  //   @    
  //   !    
  wrapper->addManipulator({0, -1});
  game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::MANIPULATOR]);

  // * * * !
  //   @    
  //   *    
  wrapper->addManipulator({2, 1});
  game.tick();
  EXPECT_EQ(0, game.num_boosters[BoosterType::MANIPULATOR]);
}

TEST(ActionTest, Drill) {
  Game game("(0,0),(2,0),(2,3),(0,3)#(0,0)#(1,1),(2,1),(2,2),(1,2)#L(0,1)");
  // .. 2
  // L# 1
  // @. 0
  // 01

  Wrapper* wrapper = game.wrappers[0].get();
  wrapper->move(Action::UP); game.tick();
  wrapper->useBooster(Action::DRILL); game.tick();
  wrapper->move(Action::RIGHT); game.tick();
  EXPECT_EQ(Point(1, 1), wrapper->pos);
  EXPECT_FALSE(game.map2d(1,1) & CellType::kObstacleBit);
  wrapper->move(Action::RIGHT); game.tick();
  EXPECT_FALSE(game.map2d(2,1) & CellType::kObstacleBit);
}

TEST(ActionTest, DISABLED_FastWheel) {
  // example-01.desc
  Game game("(0,0),(2,0),(2,4),(0,4)#(0,0)##F(0,1);L(0,2)");
  // .. 3
  // l. 2
  // f. 1
  // @. 0
  // 01

  // Wrapper* wrapper = game.wrappers[0].get();
  // EXPECT_TRUE((game.map2d(0, 1) & CellType::kWrappedBit) == 0);
  // wrapper->move(Action::UP); game.tick();
  // EXPECT_TRUE((game.map2d(0, 2) & CellType::kWrappedBit) == 0);
  // wrapper->turn(Action::CCW); game.tick();
  // EXPECT_EQ(1, game.num_boosters[BoosterType::MANIPULATOR]);
  // EXPECT_FALSE((game.map2d(0, 2) & CellType::kWrappedBit) == 0);
  // wrapper->turn(Action::CW); game.tick();

  // wrapper->move(Action::UP); game.tick();

  // wrapper->move(Action::UP); game.tick();
  // EXPECT_EQ(1, game.num_boosters[BoosterType::FAST_WHEEL]);

  // wrapper->move(Action::UP); game.tick();
  // EXPECT_EQ(1, game.num_boosters[BoosterType::DRILL]);
  // wrapper->move(Action::UP); game.tick();
  // wrapper->move(Action::UP); game.tick();
  // wrapper->move(Action::UP); game.tick();
  // wrapper->move(Action::UP); game.tick();
  // wrapper->move(Action::UP); game.tick();
  // EXPECT_EQ(Point(0, 9), wrapper->pos);

  // wrapper->move(Action::RIGHT); game.tick();
  // wrapper->move(Action::RIGHT); game.tick();
  // wrapper->move(Action::RIGHT); game.tick();
  // wrapper->move(Action::DOWN); game.tick();
  // wrapper->move(Action::DOWN); game.tick();
  // wrapper->move(Action::DOWN); game.tick();
  // EXPECT_EQ(Point(3, 6), wrapper->pos);

  // int num_unwrapped = game.map2d.num_unwrapped;
  // wrapper->useBooster(Action::DRILL); game.tick();
  // EXPECT_EQ(0, game.num_boosters[BoosterType::DRILL]);

  // wrapper->move(Action::RIGHT); game.tick();
  // EXPECT_EQ(num_unwrapped - 1, game.map2d.num_unwrapped);  // (5,6) is newly wrapped.

  // wrapper->addManipulator({-1, 0}); game.tick();
  // EXPECT_EQ(0, game.num_boosters[BoosterType::MANIPULATOR]);
  // wrapper->useBooster(Action::FAST); game.tick();
  // EXPECT_EQ(0, game.num_boosters[BoosterType::FAST_WHEEL]);
}
