#include "trajectory.h"

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
