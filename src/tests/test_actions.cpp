#include <gtest/gtest.h>

#include "action.h"
#include "booster.h"
#include "base.h"
#include "game.h"

TEST(ActionTest, AffectsOnGame) {
  // example-01.desc
  Game game("(0,0),(10,0),(10,10),(0,10)#(0,0)#(4,2),(6,2),(6,7),(4,7);(5,8),(6,8),(6,9),(5,9)#B(0,1);B(1,1);F(0,2);F(1,2);L(0,3);X(0,9)");

  Wrapper* wrapper = game.wrappers[0].get();
  wrapper->move(Action::UP); game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::MANIPULATOR]);

  EXPECT_TRUE((game.map2d(0, 2) & CellType::kWrappedBit) == 0);
  wrapper->turn(Action::CCW); game.tick();
  EXPECT_FALSE((game.map2d(0, 2) & CellType::kWrappedBit) == 0);
  wrapper->turn(Action::CW); game.tick();

  wrapper->move(Action::UP); game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::FAST_WHEEL]);

  wrapper->move(Action::UP); game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::DRILL]);

  wrapper->move(Action::UP); game.tick();
  wrapper->move(Action::UP); game.tick();
  wrapper->move(Action::UP); game.tick();
  wrapper->move(Action::UP); game.tick();
  wrapper->move(Action::UP); game.tick();
  wrapper->move(Action::UP); game.tick();
  EXPECT_EQ(Point(0, 9), wrapper->pos);
  wrapper->move(Action::UP); game.tick();
  EXPECT_EQ(Point(0, 9), wrapper->pos);

  wrapper->move(Action::RIGHT); game.tick();
  wrapper->move(Action::RIGHT); game.tick();
  wrapper->move(Action::RIGHT); game.tick();
  wrapper->move(Action::DOWN); game.tick();
  EXPECT_EQ(Point(3, 8), wrapper->pos);

  // TODO: Is {-2,-2} impossible?
  wrapper->addManipulate({-2, -2}); game.tick();
  EXPECT_EQ(0, game.num_boosters[BoosterType::MANIPULATOR]);
  wrapper->useBooster(Action::FAST); game.tick();
  EXPECT_EQ(0, game.num_boosters[BoosterType::FAST_WHEEL]);
  wrapper->useBooster(Action::DRILL); game.tick();
  EXPECT_EQ(0, game.num_boosters[BoosterType::DRILL]);
}

TEST(ActionTest, Clone) {
  // example-01.desc + mod.
  Game game("(0,0),(10,0),(10,10),(0,10)#(0,0)#(4,2),(6,2),(6,7),(4,7);(5,8),(6,8),(6,9),(5,9)#C(0,1);X(0,2)");

  Wrapper* wrapper = game.wrappers[0].get();
  //
  wrapper->move(Action::UP);
  game.tick();
  EXPECT_EQ(1, game.num_boosters[BoosterType::CLONING]);
  //
  wrapper->move(Action::UP);
  game.tick();
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