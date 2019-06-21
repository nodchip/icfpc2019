#include "wrappy.h"

Wrappy::Wrappy(Map& mp)
  : map(mp) {
  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
  point = mp.wrappy_point;
}
