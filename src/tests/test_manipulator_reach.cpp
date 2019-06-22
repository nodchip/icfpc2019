#include "../manipulator_reach.h"

#include <ostream>
#include <iostream>
#include <gtest/gtest.h>

TEST(manipulator_reach, requiredClearance) {
  // exact diagonal.
  for (int i = -1; i <= 1; i += 2) {
    for (int j = -1; j <= 1; j += 2) {
      {
        // . . *
        // . * .
        // * . .
        auto res = requiredClearance({i * 2, j * 2});
        ASSERT_EQ(res.size(), 3);
        EXPECT_EQ(res[0], Point(i * 0, j * 0));
        EXPECT_EQ(res[1], Point(i * 1, j * 1));
        EXPECT_EQ(res[2], Point(i * 2, j * 2));
      }
      {
        // . . * *
        // * * . .
        auto res = requiredClearance({i * 3, j * 1});
        ASSERT_EQ(res.size(), 4);
        EXPECT_EQ(res[0], Point(i * 0, j * 0));
        EXPECT_EQ(res[1], Point(i * 1, j * 0));
        EXPECT_EQ(res[2], Point(i * 2, j * 1));
        EXPECT_EQ(res[3], Point(i * 3, j * 1));
      }
    }
  }

  // nonexact diagonal.
  // . * *
  // * * .
  for (int i = -1; i <= 1; i += 2) {
    for (int j = -1; j <= 1; j += 2) {
      auto res = requiredClearance({i * 2, j * 1});
      ASSERT_EQ(res.size(), 4);
      EXPECT_EQ(res[0], Point(i * 0, j * 0));
      EXPECT_EQ(res[1], Point(i * 1, j * 0));
      EXPECT_EQ(res[2], Point(i * 1, j * 1));
      EXPECT_EQ(res[3], Point(i * 2, j * 1));
    }
  }
}

TEST(manipulator_reach, absolutePositionOfReachableManipulators) {
  // Y^ 
  //  |
  //  +---------+
  // x|o x # o o|x  
  //  |. . o @ .|
  //  *---------+--->
  //  ^             X
  //  (0,0)
  Map2D map(5, 2, {
    0, 0, 0, 0, 0, // y=0
    0, 0, 2, 0, 0, // y=1
  });
  Point wrappy {3, 0};
  std::vector<std::pair<Point, bool>> manipulators = {
    {{-1, 0}, true},
    {{-2, 1}, false},
    {{-3, 1}, true},
    {{-4, 1}, false},
    {{0, 1}, true},
    {{1, 1}, true},
    {{2, 1}, false},
  };
  for (auto test : manipulators) {
    auto res = absolutePositionOfReachableManipulators(map, wrappy, {test.first});
    if (test.second) {
      EXPECT_EQ(res.size(), 1ull);
      if (res.size() == 1) {
        EXPECT_EQ(res[0], wrappy + test.first);
      }
    } else {
      EXPECT_EQ(res.size(), 0ull);
    }
  }
}
