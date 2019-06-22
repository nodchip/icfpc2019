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
  os << "pos : (" << traj.pos.x << "," << traj.pos.y << ") ";
  os << "dist : " << traj.distance << " ";
  os << "last_move : "<<traj.last_move;
  return os;
}

char Direction2Char(const Direction d){
  switch (d) {
  case Direction::W: return 'W';
  case Direction::S: return 'S';
  case Direction::A: return 'A';
  case Direction::D: return 'D';
  }
  return '!'; // unreachable!!
}
