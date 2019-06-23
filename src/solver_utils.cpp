#include "game.h"

#include <queue>
#include <iostream>

using namespace std;

namespace utils {
  int countUnWrappedArea(const Game &game, const Point &from) {
    static const int kMask = CellType::kObstacleBit | CellType::kWrappedBit;
    const int H = game.map2d.H;
    const int W = game.map2d.W;
    int count(0);
    queue<Point> que;
    que.push(from);
    Point d[4] = {Point(0, 1), Point(0, -1), Point(1, 0), Point(-1, 0)};
    auto &map2d(game.map2d);
    vector<vector<bool>> visited(H, vector<bool>(W, false));

    while (!que.empty()) {
      auto p = que.front();
      que.pop();
      cout << p << endl;
      if (map2d.isInside(p) && (map2d(p) & kMask) == 0 && !visited[p.y][p.x]) {
        ++count;
        visited[p.y][p.x] = true;
        for (int i = 0; i < 4; ++i) {
          que.push(p+d[i]);
        }
      }
    }
    return count;
  }

  vector<vector<double>> getGloryMap(const Game &game) {
    static const int kMask = CellType::kObstacleBit;
    const int H = game.map2d.H;
    const int W = game.map2d.W;
    static const int numIteration(H + W);
    vector<vector<vector<double>>> value(2, vector<vector<double>>(H, vector<double>(W, 0.0)));
    auto &map2d(game.map2d);
    for (int y = 0; y < H; ++y) {
      for (int x = 0; x < W; ++x) {
        if ((map2d(x, y) & kMask) == 0) {
          value[0][y][x] = 1.0;
        }
      }
    }
    Point d[4] = {Point(0, 1), Point(0, -1), Point(1, 0), Point(-1, 0)};
    for (int i = 0; i < numIteration; ++i) {
      auto &vf(value[i % 2]);
      auto &vt(value[(i + 1) % 2]);
      for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
          Point c(x, y);
          int count(0);
          double sum(0.0);
          for (int j = 0; j < 4; ++j) {
            if (map2d.isInside(c + d[j]) && (map2d(x, y) & kMask) == 0) {
              count++;
              sum += vf[y+d[j].y][x+d[j].x] * 0.1;
            }
            vt[y][x] = vf[y][x] * (1.0 - 0.1 * count) + sum;
          }
        }
      }
    }
    return value[numIteration%2];
  }

//#define DEBUG_PRINT
  void processCurrentGloryMap(const Game &game, const vector<vector<double>> &iMap, vector<vector<double>> &oMap) {
    static const int kMask = CellType::kObstacleBit | CellType::kWrappedBit;
    static const int kMask2 = CellType::kObstacleBit;
    const int H = game.map2d.H;
    const int W = game.map2d.W;
    static const int numIteration(std::min<int>((H + W) / 2, 30));
    auto &map2d(game.map2d);
    vector<vector<vector<double>>> value(2, vector<vector<double>>(H, vector<double>(W, 0.0)));
    for (int y = 0; y < H; ++y) {
      for (int x = 0; x < W; ++x) {
        if ((map2d(x, y) & kMask) == 0) {
          value[0][y][x] = 1.0 / iMap[y][x];
        }
      }
    }
#ifdef DEBUG_PRINT
    cout << game.time << endl;
    for (const auto &l : value[0]) {
      for (const auto &e : l) {
        cout << e << ", ";
      }
      cout << endl;
    }
#endif
    Point d[4] = {Point(0, 1), Point(0, -1), Point(1, 0), Point(-1, 0)};
    for (int i = 0; i < numIteration; ++i) {
      auto &vf(value[i % 2]);
      auto &vt(value[(i + 1) % 2]);
      for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
          Point c(x, y);
          int count(0);
          double sum(0.0);
          for (int j = 0; j < 4; ++j) {
            if (map2d.isInside(c + d[j])) {
              if ((map2d(x, y) & kMask) == 0) {
                sum += vf[y+d[j].y][x+d[j].x] * 0.2;
              } else if((map2d(x, y) & kMask2) == 0) {
                sum += vf[y+d[j].y][x+d[j].x] / numIteration;
              }
            }
            vt[y][x] = vf[y][x] + sum;
          }
        }
      }
    }
    for (int y = 0; y < H; ++y) {
      for (int x = 0; x < W; ++x) {
        oMap[y][x] = value[numIteration%2][y][x];
      }
    }
  }

  vector<Point> findNearestWay(const Game &game, const Point &from, const Point &to) {
      return vector<Point>();
  }

}; // namespace utils