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
  std::cout<<game<<std::endl;
  {
    std::vector<Trajectory> trajs = map_parse::findTrajectory(game, {0,0}, {0,4}, DISTANCE_INF);
    for(auto t : trajs){
      std::cout<<t<<std::endl;
    }
    EXPECT_EQ(10, trajs.size());
  }
  {
    std::vector<Trajectory> trajs = map_parse::findTrajectory(game, {0,0}, {5,0}, DISTANCE_INF);
    for(auto t : trajs){
      std::cout<<t<<std::endl;
    }

    EXPECT_EQ(9, trajs.size());
  }
  {
    std::vector<Trajectory> trajs = map_parse::findNearestUnwrapped(game, {3,1}, DISTANCE_INF);
    for(auto t : trajs){
      std::cout<<t<<std::endl;
    }

    EXPECT_EQ(2, trajs.size());
  }
}
