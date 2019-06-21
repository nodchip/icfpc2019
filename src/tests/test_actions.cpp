#include "../game.h"

#include <ostream>
#include <iostream>
#include <gtest/gtest.h>

#include "../base.h"

TEST(actions, Game_test_actions) {
  // example-01.desc
  Game game("(0,0),(10,0),(10,10),(0,10)#(0,0)#(4,2),(6,2),(6,7),(4,7);(5,8),(6,8),(6,9),(5,9)#B(0,1);B(1,1);F(0,2);F(1,2);L(0,3);X(0,9)");

  constexpr int W = CellType::kObstacleBit;
  constexpr int B = CellType::kBoosterManipulatorBit;
  constexpr int F = CellType::kBoosterFastWheelBit;
  constexpr int L = CellType::kBoosterDrillBit;
  constexpr int R = CellType::kBoosterTeleportBit;
  constexpr int X = CellType::kBoosterUnknownXBit;
  Map2D ground_truth(10, 10, {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    B, B, 0, 0, 0, 0, 0, 0, 0, 0,
    F, F, 0, 0, W, W, 0, 0, 0, 0,
    L, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, W, 0, 0, 0, 0,
    X, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  });
  std::cout << game.map2d << std::endl;
  std::cout << ground_truth << std::endl;
  EXPECT_EQ(game.map2d, ground_truth);

  game.move(Game::UP);
  game.move(Game::UP); // B
  game.move(Game::UP); // F
  game.move(Game::UP); // L
  game.move(Game::UP);
  game.move(Game::UP);
  game.move(Game::UP);
  game.move(Game::UP);
  game.move(Game::UP); // on X
  game.move(Game::RIGHT);
  game.move(Game::RIGHT);
  game.move(Game::RIGHT);
  game.move(Game::DOWN);
  game.turn(Game::CW);
  game.turn(Game::CCW);
  game.addManipulate({-2, -2});
  game.useBooster(Game::FAST);
  game.useBooster(Game::DRILL);

  //std::cout << game.map2d << std::endl;
  //std::cout << game.map2d.W << "x" << game.map2d.H << std::endl;
  std::cout << game << std::endl;
}