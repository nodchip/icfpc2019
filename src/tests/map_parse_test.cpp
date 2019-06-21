#include "../map_parse.h"

#include <gtest/gtest.h>
#include <iostream>

TEST(MapParseTest, TrajectoryTest) {
  std::vector<std::string> test_map {
    ". . ..", 
    "###. .",
    "... ##",
    " #  #.",
    "..#  .",
  };
  Game game(test_map);
  {
    Trajectory trajectory = map_parse::find_trajectory(game, {0,0}, {0,4}, DISTANCE_INF);
    EXPECT_EQ(10, trajectory.distance);
  }
  {
    Trajectory trajectory = map_parse::find_trajectory(game, {0,0}, {5,0}, DISTANCE_INF);
    EXPECT_EQ(9, trajectory.distance);
  }

  {
    Trajectory trajectory = map_parse::find_nearest_unwrapped(game, {3,1}, DISTANCE_INF);
    EXPECT_EQ(2, trajectory.distance);
  }
}
