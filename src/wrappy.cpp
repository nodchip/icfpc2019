#include "wrappy.h"

#include <ostream>

Wrappy::Wrappy(Map& mp)
  : map(mp) {
  manipulators.push_back(Point {1, 0});
  manipulators.push_back(Point {1, 1});
  manipulators.push_back(Point {1, -1});
  point = mp.wrappy_point;
}

std::ostream& operator<<(std::ostream& os, const Wrappy& wrap) {
  os << wrap.map << "\n"
     << "Boosters: F(" << wrap.fast_wheels << ") "
     << "L(" << wrap.drills << ")\n";
  if (wrap.time_fast_wheels > 0) {
    os << " Speedup (" << wrap.time_fast_wheels << ")\n";
  }
  if (wrap.time_drill > 0) {
    os << " Drill (" << wrap.time_drill << ")\n";
  }

  return os;
}
