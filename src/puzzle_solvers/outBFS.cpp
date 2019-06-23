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
#include "solver_registry.h"

namespace{

  struct XorShift {
	unsigned x, y, z, w;
	XorShift() { x = 123456789; y = 362436069; z = 521288629; w = 88675123; }
	XorShift(int _w) { x = 123456789; y = 362436069; z = 521288629; w = _w; }
	void setSeed() { x = 123456789; y = 362436069; z = 521288629; w = 88675123; }
	void setSeed(int _w) { x = 123456789; y = 362436069; z = 521288629; w = _w; }
	unsigned nextUInt() {
		unsigned t = (x ^ (x << 11));
		x = y; y = z; z = w;
		return (w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)));
	}
	unsigned nextUInt(unsigned mod) {
		unsigned t = (x ^ (x << 11));
		x = y; y = z; z = w;
		w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
		return w % mod;
	}
	unsigned nextUInt(unsigned l, unsigned r) {
		unsigned t = (x ^ (x << 11));
		x = y; y = z; z = w;
		w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
		return w % (r - l + 1) + l;
	}
	double nextDouble() {
		return double(nextUInt()) / UINT_MAX;
	}
} rnd;

}

PuzzleSolution outBFS(PuzzleSolverParam param, Puzzle puzzle)
{
  int tSize = puzzle.tSize, H = tSize + 2, W = tSize + 2; // 外枠
  auto &iSqs = puzzle.iSqs;
  auto oSqs = puzzle.oSqs;

  const int OUT = 0;
  const int IN = 1;
  const int OBSTACLE = 2;
  std::vector<std::vector<int>> board(H, std::vector<int>(W, 0));
  for(int i = 1; i <= H - 2; i++) for(int j = 1; j <= W - 2; j++) board[i][j] = IN;
  for(Point p : iSqs) board[p.y + 1][p.x + 1] = OBSTACLE;

  // OUT までの最短経路
  auto getPath = [&](int i, int j) {
    const int di[] = {0, -1, 0, 1};
    const int dj[] = {1, 0, -1, 0};
    std::vector<std::vector<int>> dist(H, std::vector<int>(W, INT_MAX));
    std::vector<std::vector<bool>> visited(H, std::vector<bool>(W, false));
    //std::vector<int> dist(H * W, INT_MAX);
    std::queue<Point> qu;
    qu.push(Point(j, i));
    dist[i][j] = 0;
    while(!qu.empty()){
      Point p = qu.front(); qu.pop();
      int i = p.y, j = p.x;
      for(int d = 0; d < 4; d++){
        int ni = i + di[d];
        int nj = j + dj[d];
        if(ni < 0 || ni >= H || nj < 0 || nj >= W) continue;
        if(board[ni][nj] == OBSTACLE) continue;
        if(dist[ni][nj] > dist[i][j] + 1){
          dist[ni][nj] = dist[i][j] + 1;
          qu.push(Point(nj, ni));
        }
        if(board[ni][nj] == OUT){
          Polygon poly;
          int nd = dist[ni][nj];
          poly.emplace_back(nj, ni);
          nd--;
          for(; nd >= 0; nd--){
            for(int dd = 0; dd < 4; dd++){
              int nni = ni + di[dd], nnj = nj + dj[dd];
              if(nni < 0 || nni >= H || nnj < 0 || nnj >= W || dist[nni][nnj] != nd) continue;
              ni = nni;
              nj = nnj;
              poly.emplace_back(nj, ni);
            }
          }
          reverse(poly.begin(), poly.end());
          //std::cerr << poly.size() << std::endl;
          return poly;
        }
      }
    }
    return Polygon();
  };

  // auto dumpBoard = [&](const Map2D& map2d){
  //   char cvt[] = {'0', '1', '2'};
  //   for(int i = 0; i < tSize; i++){
  //     for(int j = 0; j < tSize; j++){
  //       std::cerr << map2d.data[i * tSize + j];
  //     }
  //     std::cerr << std::endl;
  //   }
  // };

  for(Point p : oSqs){
    int i = p.y + 1;
    int j = p.x + 1;
    Polygon path = getPath(i, j);
    //std::cerr << path.size() << std::endl;
    for(Point q : path){
      board[q.y][q.x] = OUT;
    }
  }

  Map2D map2d(tSize, tSize);
  for(int i = 0; i < tSize; i++){
    for(int j = 0; j < tSize; j++){
      if(board[i + 1][j + 1] != OUT) map2d.data[i * tSize + j] = 1;
    }
  }

  //dumpBoard(map2d);
  
  Polygon fine_polygon;
  assert(parsePolygon(fine_polygon, map2d, 1));
  //std::cerr << fine_polygon.size() << std::endl;
  Polygon simple_polygon = simplifyPolygon(fine_polygon);
  //std::cerr << simple_polygon.size() << std::endl;

  // // vMax 条件を満たさない場合はこの解法では修正が厳しい
  assert(simple_polygon.size() <= puzzle.vMax);

  // くし型
  std::vector<std::vector<bool>> iSqsMat(tSize, std::vector<bool>(tSize, false));
  for(Point p : iSqs){
    iSqsMat[p.y][p.x] = true;
  }
  int y = 0, x = 1;
  while(simple_polygon.size() < puzzle.vMin && x < tSize - 1){
    if(iSqsMat[y][x]) continue;
    // くしを追加すると頂点が 4 増える
    bool flag = true;
    for(int dy = 0; dy <= 1; dy++){
      for(int dx = -1; dx <= 1; dx++){
        //if(board[dy][x + dx] == OUT) flag = false;
        if(map2d.data[(y + dy) * tSize + (x + dx)] == 0) flag = false;
      }
    }
    if(flag){
      map2d.data[y * tSize + x] = 0;
      parsePolygon(fine_polygon, map2d, 1);
      simple_polygon = simplifyPolygon(fine_polygon);
    }
    x++;
  }

  y = tSize - 1; x = 1;
  while(simple_polygon.size() < puzzle.vMin && x < tSize - 1){
    if(iSqsMat[y][x]) continue;
    // くしを追加すると頂点が 4 増える
    bool flag = true;
    for(int dy = -1; dy <= 0; dy++){
      for(int dx = -1; dx <= 1; dx++){
        //if(board[dy][x + dx] == OUT) flag = false;
        if(map2d.data[(y + dy) * tSize + (x + dx)] == 0) flag = false;
      }
    }
    if(flag){
      map2d.data[y * tSize + x] = 0;
      parsePolygon(fine_polygon, map2d, 1);
      simple_polygon = simplifyPolygon(fine_polygon);
    }
    x++;
  }

  //dumpBoard(map2d);
  //std::cerr << simple_polygon.size() << std::endl;
  assert(simple_polygon.size() >= puzzle.vMin);

  //exit(1);
  
  PuzzleSolution solution;
  solution.wall = simplifyPolygon(fine_polygon);

  // scatter wrappy and boosters
  auto findUnwrappedPoints = [&](int n){
    Polygon poly;
    for(int i = 0; i < tSize; i++){
      for(int j = 0; j < tSize; j++){
        if(map2d.data[i * tSize + j] == 1) {
          poly.push_back(Point(j, i));
        }
        if(poly.size() == n) break;
      }
      if(poly.size() == n) break;
    }
    return poly;
  };

  {
    int bNum = puzzle.mNum;
    int fNum = puzzle.fNum;
    int lNum = puzzle.dNum;
    int rNum = puzzle.rNum;
    int cNum = puzzle.cNum;
    int xNum = puzzle.xNum;
    Polygon pts = findUnwrappedPoints(bNum + fNum + lNum + rNum + cNum + xNum + 1);
    for(int i = 0; i < bNum; i++) {solution.Bs.push_back(pts.back()); pts.pop_back(); }
    for(int i = 0; i < fNum; i++) {solution.Fs.push_back(pts.back()); pts.pop_back(); }
    for(int i = 0; i < lNum; i++) {solution.Ls.push_back(pts.back()); pts.pop_back(); }
    for(int i = 0; i < rNum; i++) {solution.Rs.push_back(pts.back()); pts.pop_back(); }
    for(int i = 0; i < cNum; i++) {solution.Cs.push_back(pts.back()); pts.pop_back(); }
    for(int i = 0; i < xNum; i++) {solution.Xs.push_back(pts.back()); pts.pop_back(); }
    solution.wrapper = pts.back();
  }

  // Map2D recon(tSize, tSize);
  // fillPolygon(recon, solution.wall, 1);
  // std::vector<std::string> dmp = dumpMapString(recon, Polygon());
  // for(auto s : dmp) std::cerr << s << std::endl;

  return solution;
}

REGISTER_PUZZLE_SOLVER("outBFS", outBFS);
