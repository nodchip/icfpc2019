// 1pxの壁以外は基本的にroom内にする
// oSqsを壁につなげるために伸ばしていくが、その過程で部屋を分断しないようにする
#include <memory>
#include <string>
#include <queue>
#include <functional>
#include <iostream>
#include <climits>
#include <map>
#include <vector>
#include <set>
#include <cassert>

#include "puzzle.h"
#include "fill_polygon.h"
#include "map_parse.h"
#include "solver_registry.h"
#include "solver_helper.h"

PuzzleSolution minimumExcludePuzzleSolver(PuzzleSolverParam param, Puzzle puzzle) {
  int W = 0, H = 0;
  for (auto p : puzzle.iSqs) {
    W = std::max(W, p.x);
    H = std::max(H, p.y);
  }
  for (auto p : puzzle.oSqs) {
    W = std::max(W, p.x);
    H = std::max(H, p.y);
  }
  W += 1;
  H += 1;
  std::cerr << "W=" << W << " H=" << H << std::endl;

  constexpr int ROOM = 0;
  constexpr int WALL = 1;
  constexpr int INCLUDE = 2;
  // 1px wall.
  Point offset {1, 1};
  Map2D work(W + 2, H + 2, ROOM);
  for (auto p : puzzle.iSqs) {
    work(offset + p) |= INCLUDE;
  }
  for (int y = 0; y < H; ++y) {
    work(0, offset.y + y) = WALL;
    work(work.W - 1, offset.y + y) = WALL;
  }
  for (int x = 0; x < W; ++x) {
    work(offset.y + x, 0) = WALL;
    work(offset.y + x, work.H - 1) = WALL;
  }

  for (auto p : puzzle.oSqs) {
    if (work(offset + p) == WALL) { continue; }
    // find nearest wall.
    auto path = shortestPathByMaskBFS(work,
      0b11, 0b00, // room, not include
      0b11, 0b01, // wall, not include
      offset + p);
    assert (!path.empty());
    for (auto way_plus_offset : path) {
      assert ((work(way_plus_offset) & INCLUDE) == 0);
      work(way_plus_offset) = WALL;
    }
    //std::cout << work.toString(true, true, 1) << std::endl;
  }
  work = work.slice(offset.x, work.W - offset.x, offset.x, work.H - offset.x);
  std::cerr << work.toString(true, true, 1) << std::endl;

  // remove include flag before converting into polygon.
  Map2D room_map = work; // only 0:room or 1:wall.
  for (int y = 0; y < work.H; ++y)
    for (int x = 0; x < work.W; ++x)
      room_map(x, y) &= ~INCLUDE;
  
  // 4-connected (NOTE: work can be non 4-connected due to INCLUDE points.)
  assert (isConnected4(room_map));
  assert (disjointConnectedComponentsByMask(room_map, 1, 0).size() == 1);

  // increase vertices.
  Polygon fine_polygon;
  Polygon simple_polygon;
  while (true) {
    parsePolygon(fine_polygon, room_map, ROOM);
    simple_polygon = simplifyPolygon(fine_polygon);
    const int n_verts = simple_polygon.size();
    //std::cout << n_verts << " .. " << puzzle.vMin << std::endl;
    if (n_verts < puzzle.vMin) {
      // add a pixel to a wall.
      std::vector<Point> wall_neighbor;
      for (int y = 0; y < room_map.H; ++y) {
        for (int x = 0; x < room_map.W; ++x) {
          // never select from iSqs
          if (work(x, y) & INCLUDE) continue;
          // all border points.
          bool is_wall_neighbor = x == 0 || x + 1 == room_map.W || y == 0 || y + 1 == room_map.H;
          // or 4-connected to a wall.
          if (!is_wall_neighbor) {
            for (auto o : neighbors4) {
              if (room_map(Point {x, y} + o) == WALL) {
                is_wall_neighbor = true;
              }
            }
          }
          if (is_wall_neighbor) {
            wall_neighbor.push_back({x, y});
          }
        }
      }
      assert (!wall_neighbor.empty());

      // pick random candidate and evaluate.
      Point new_wall;
      while (true) {
        new_wall = wall_neighbor[rand() % wall_neighbor.size()];
        room_map(new_wall) = WALL;
        // it divides the map.
        if (disjointConnectedComponentsByMask(room_map, 1, 0).size() > 1) {
          room_map(new_wall) = ROOM;
          continue;
        }
        // bad connectivity.
        if (!isConnected4(room_map)) {
          room_map(new_wall) = ROOM;
          continue;
        }
        // OK.
        break;
      }
      // accept wall.
      room_map(new_wall) = WALL;
      work(new_wall) = WALL;
    } else {
      break;
    }
  }
  std::cerr << room_map.toString(true, true, 1) << std::endl;

  PuzzleSolution solution;
  solution.wall = simple_polygon;

  Map2D occupiedByPlacement(work.W, work.H, 0);
  auto popRandomPlacementPosition = [&]() {
    int x = 0, y = 0;
    do {
      x = rand() % room_map.W;
      y = rand() % room_map.H;
    } while (occupiedByPlacement(x, y) || room_map(x, y) != ROOM);
    occupiedByPlacement(x, y) = 1;
    return Point {x, y};
  };

  {
    int bNum = puzzle.mNum;
    int fNum = puzzle.fNum;
    int lNum = puzzle.dNum;
    int rNum = puzzle.rNum;
    int cNum = puzzle.cNum;
    int xNum = puzzle.xNum;
    for(int i = 0; i < bNum; i++) solution.Bs.push_back(popRandomPlacementPosition());
    for(int i = 0; i < fNum; i++) solution.Fs.push_back(popRandomPlacementPosition());
    for(int i = 0; i < lNum; i++) solution.Ls.push_back(popRandomPlacementPosition());
    for(int i = 0; i < rNum; i++) solution.Rs.push_back(popRandomPlacementPosition());
    for(int i = 0; i < cNum; i++) solution.Cs.push_back(popRandomPlacementPosition());
    for(int i = 0; i < xNum; i++) solution.Xs.push_back(popRandomPlacementPosition());
    solution.wrapper = popRandomPlacementPosition();
  }

  return solution;
}

REGISTER_PUZZLE_SOLVER("minimum_exclude", minimumExcludePuzzleSolver);
