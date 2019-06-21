#include "trajectory.h"

std::ostream& operator<<(std::ostream& os, const Trajectory& traj) {
  os << "from : ("<<traj.from.first<<","<<traj.from.second<<") ";
  os << "to : ("<<traj.to.first<<","<<traj.to.second<<") ";
  os << "dist : "<<traj.distance<<" ";
  os << "path : [";
  for(auto p : traj.path){
    os << p <<",";
  }
  os<<"]";
  return os;
  
}
