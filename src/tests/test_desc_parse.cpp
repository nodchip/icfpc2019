#include "../game.h"

#include <ostream>
#include <iostream>
#include <gtest/gtest.h>

#include "../base.h"

TEST(GameTest, ParseDesc) {
  // example-01.desc
  Game game("(0,0),(10,0),(10,10),(0,10)#"
            "(0,0)#"
            "(4,2),(6,2),(6,7),(4,7);"
            "(5,8),(6,8),(6,9),(5,9)#"
            "B(0,1);B(1,1);F(0,2);F(1,2);L(0,3);X(0,9)");

  constexpr int P = CellType::kWrappedBit;
  constexpr int W = CellType::kObstacleBit;
  constexpr int B = CellType::kBoosterManipulatorBit;
  constexpr int F = CellType::kBoosterFastWheelBit;
  constexpr int L = CellType::kBoosterDrillBit;
  constexpr int X = CellType::kSpawnPointBit;
  Map2D ground_truth(10, 10, {
    P, P, 0, 0, 0, 0, 0, 0, 0, 0,
    B, B|P, 0, 0, 0, 0, 0, 0, 0, 0,
    F, F, 0, 0, W, W, 0, 0, 0, 0,
    L, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, W, W, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, W, 0, 0, 0, 0,
    X, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  });
  EXPECT_EQ(game.map2d, ground_truth);
}
