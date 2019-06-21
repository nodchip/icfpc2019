#include "trajectory.h"

std::ostream &operator<<(std::ostream &os, const Direction &d) {
  switch (d) {
  case Direction::W: os << "W"; break;
  case Direction::S: os << "S"; break;
  case Direction::A: os << "A"; break;
  case Direction::D: os << "D"; break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const Trajectory &traj) {
  os << "from : (" << traj.from.x << "," << traj.from.y << ") ";
  os << "to : (" << traj.to.x << "," << traj.to.y << ") ";
  os << "dist : " << traj.distance << " ";
  os << "path : [";
  for (auto p : traj.path) {
    os << p << ",";
  }
  os << "]";
  return os;
}
