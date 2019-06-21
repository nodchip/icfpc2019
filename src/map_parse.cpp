#include <queue>

#include "map_parse.h"


// 経路の価値の比較。評価関数に相当する
bool traj_evaluator::comp_traj(const Trajectory &t1, const Trajectory &t2){
  return t1.distance < t2.distance;
}


// 経路の価値の比較。評価関数に相当する
const bool comp_traj(const Trajectory &t1, const Trajectory &t2){
  return t1.distance < t2.distance;
}

// 経路の価値の比較。評価関数に相当する
bool overwrite(const Trajectory &before, const Trajectory after){
  return comp_traj(before, after);
}

void set_traj(Trajectory &traj, const Point from_in, const Point to_in, const int distance_in, const bool use_dig, const std::vector<Direction> path_in){
  traj.from = from_in;
  traj.to = to_in;
  traj.distance = distance_in;
  traj.path = path_in;
  traj.use_dig = use_dig;
}


Trajectory map_parse::find_trajectory(const Game &game, const Point from, const Point to, const int max_dist){
  Trajectory traj_map[MAP_XMAX][MAP_YMAX];
  std::priority_queue<Trajectory, std::vector<Trajectory>, decltype(&comp_traj)> que(&comp_traj);
  set_traj(traj_map[from.first][from.second], from, from, 0, false, std::vector<Direction>(0));

  que.push(traj_map[from.first][from.second]);
  while(1){
    if (que.empty()){
      break;
    }
    Trajectory traj; // = que.pop();

    if(traj.distance > max_dist){
      continue;
    }
    
    {
      const int x_try = 0;
      const int y_try = 0;
      
      if(game.map[x_try][y_try] == '#'){
	// todo write drill
	continue;
      }
      
      Direction dir = Direction(W); 
      Trajectory traj_try = traj;
      traj_try.path.push_back(dir);
      traj_try.distance += 1;
      traj_try.to = {x_try, y_try};
      if(overwrite(traj_map[x_try][y_try], traj_try)){
	traj_map[x_try][y_try] = traj_try;
	que.push(traj_try);
      }
    }
  }
  return traj_map[to.first][to.second];
}

Trajectory map_parse::find_nearest_unwrapped(const Game &game, const Point from, const int max_dist){
  Trajectory traj_map[MAP_XMAX][MAP_YMAX];
  std::priority_queue<Trajectory, std::vector<Trajectory>, decltype(&comp_traj)> que(&comp_traj);
  set_traj(traj_map[from.first][from.second], from, from, 0, false, std::vector<Direction>(0));

  int nearest = DISTANCE_INF;
  Point nearest_point = {-1, -1};
  
  while(1){
    if (que.empty()){
      break;
    }
    Trajectory traj; // = que.pop();

    if(traj.distance > max_dist || traj.distance > nearest){
      continue;
    }
    
    {
      const int x_try = 0;
      const int y_try = 0;
      
      if(game.map[x_try][y_try] == '#'){
	// todo write drill
	continue;
      }
      
      Direction dir = Direction(W); 
      Trajectory traj_try = traj;
      traj_try.path.push_back(dir);
      traj_try.distance += 1;
      traj_try.to = {x_try, y_try};
      if(overwrite(traj_map[x_try][y_try], traj_try)){
	traj_map[x_try][y_try] = traj_try;
	if(game.map[x_try][y_try] == '.' && traj_try.distance < nearest){
	  nearest_point = {x_try, y_try};
	}else{
	  que.push(traj_try);
	}
      }
    }
  }
  return traj_map[nearest_point.first][nearest_point.second];
}

void map_parse::test_map_parse(){
  std::vector<std::string> test_map;
  test_map.resize(5);
  test_map[0] = ". . ..";
  test_map[1] = "###. .";
  test_map[2] = "... ##";
  test_map[3] = " #  #.";
  test_map[4] = "..#   ";
  Game game(test_map);
  Trajectory traj = map_parse::find_trajectory(game, {0,0}, {3,0}, DISTANCE_INF);
  Trajectory traj2 = map_parse::find_nearest_unwrapped(game, {4,3}, DISTANCE_INF);
}
