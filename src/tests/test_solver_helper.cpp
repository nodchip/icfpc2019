#include "../solver_helper.h"

#include <gtest/gtest.h>
#include <iostream>

TEST(SolverHelperTest, disjointConnectedComponentByMask) {
  // 2:obstacle, 1:wrapped, 0:empty
  Map2D map(8, 8, {
    2, 0, 1, 0, 0, 0, 0, 0,
    2, 0, 1, 1, 2, 2, 2, 0,
    2, 2, 1, 1, 2, 0, 2, 0,
    1, 1, 1, 1, 2, 2, 2, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0,
  });

  auto res = disjointConnectedComponentsByMask(map, 0b11, 0b00);
  EXPECT_EQ(4, res.size());
  int count = 0;
  for (auto cc : res) {
    for (auto p : cc) {
      EXPECT_EQ(0, map(p));
      ++count;
    }
  }
  EXPECT_EQ(39, count);

  std::cout << res.size() << std::endl;
  for (auto cc : res) {
    std::cout << "CC[" << cc.size() << "] ";
    for (auto p : cc) {
      std::cout << p;
    }
    std::cout << std::endl;
  }
}

TEST(SolverHelperTest, findFCRoute) {
  constexpr int C = CellType::kBoosterCloningBit;
  constexpr int F = CellType::kBoosterFastWheelBit;
  constexpr int I = CellType::kObstacleBit;
  Map2D map(10, 10, {
    0, 0, I, 0, 0, 0, 0, 0, 0, 0,
    0, 0, I, 0, 0, 0, 0, 0, 0, 0,
    0, 0, I, 0, I, I, I, 0, 0, 0,
    0, 0, I, 0, I, C, I, 0, 0, 0,
    0, 0, I, 0, I, 0, I, 0, 0, 0,
    0, 0, I, 0, 0, 0, I, 0, 0, 0,
    0, 0, I, I, I, I, I, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, F, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  });
  auto res = findGoodFCRoute(map, {1, 9});
  EXPECT_TRUE(bool(res));

  if (res) {
    std::cout << res->F_pos << std::endl;
    std::cout << res->C_pos << std::endl;
    std::cout << res->time_cost << std::endl;
  }
}