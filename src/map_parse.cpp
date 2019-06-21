#include <queue>

#include "map_parse.h"


// 経路の価値の比較。評価関数に相当する
// todo 何らかの評価関数に従って経路をソートしながら幅優先探索するという実装をしたい。どのように書くのが良いだろうか
bool traj_evaluator::comp_traj(const Trajectory &t1, const Trajectory &t2){
  return t1.distance < t2.distance;
}


// 経路の価値の比較。評価関数に相当する
const bool comp_traj(const Trajectory &t1, const Trajectory &t2){
  return t1.distance < t2.distance; // とりあえず距離が短いほうが偉いとする
}

// 経路の価値の比較。評価関数に相当する
bool overwrite(const Trajectory &before, const Trajectory after){
  return comp_traj(before, after); // とりあえず距離が短いほうが偉いとする
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
    Trajectory traj = que.top();
    que.pop();
    if(traj.distance > max_dist){
      continue;
    }
    
    auto try_expand = [&](Direction dir){
      const int x_try =  (dir == Direction(W) || dir == Direction(S)) ? traj.to.first : dir == Direction(A) ? traj.to.first - 1 : traj.to.first + 1;
      const int y_try =  (dir == Direction(A) || dir == Direction(D)) ? traj.to.second : dir == Direction(W) ? traj.to.second - 1 : traj.to.second + 1;

      if(x_try > MAP_XMAX -1 || x_try < 0 || y_try > MAP_YMAX -1 || y_try < 0){
	return;
      }
      if(game.map[x_try][y_try] == '#'){
	// todo write drill
	return;
      }
      
      Trajectory traj_try = traj;
      traj_try.path.push_back(dir);
      traj_try.distance += 1;
      traj_try.to = {x_try, y_try};
      if(overwrite(traj_map[x_try][y_try], traj_try)){
	traj_map[x_try][y_try] = traj_try;
	que.push(traj_try);
      }
    };

    try_expand(Direction(W));
    try_expand(Direction(A));
    try_expand(Direction(S));
    try_expand(Direction(D));
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
    Trajectory traj = que.top();
    que.pop();
    
    if(traj.distance > max_dist || traj.distance > nearest){
      continue;
    }
    
    auto try_expand = [&](Direction dir){
      const int x_try =  (dir == Direction(W) || dir == Direction(S)) ? traj.to.first : dir == Direction(A) ? traj.to.first - 1 : traj.to.first + 1;
      const int y_try =  (dir == Direction(A) || dir == Direction(D)) ? traj.to.second : dir == Direction(W) ? traj.to.second - 1 : traj.to.second + 1;
      
      if(x_try > MAP_XMAX -1 || x_try < 0 || y_try > MAP_YMAX -1 || y_try < 0){
	return;
      }
      if(game.map[x_try][y_try] == '#'){
	// todo write drill
	return;
      }
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
    };

    try_expand(Direction(W));
    try_expand(Direction(A));
    try_expand(Direction(S));
    try_expand(Direction(D));
  }

  return traj_map[nearest_point.first][nearest_point.second];
}
