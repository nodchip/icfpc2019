#include "../map_parse.h"

#include <gtest/gtest.h>

TEST(MapParseTest, TrajectoryTest) {
  std::vector<std::string> test_map {
    ". . ..", 
    "###. .",
    "... ##",
    " #  #.",
    "..#   ",
  };
  Game game(test_map);
  {
    Trajectory trajectory = map_parse::find_trajectory(game, {0,0}, {3,0}, DISTANCE_INF);
    EXPECT_EQ(9, trajectory.distance);
  }
  {
    Trajectory trajectory = map_parse::find_nearest_unwrapped(game, {4,3}, DISTANCE_INF);
    EXPECT_EQ(3, trajectory.distance);
  }
}
