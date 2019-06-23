#include "puzzle.h"
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cctype>

#include "fill_polygon.h"

constexpr int BLANK = 0;
constexpr int IN = 1;
constexpr int OUT = 2;

bool Puzzle::validateSolution(const PuzzleSolution& solution) const {
  std::cerr << "validateSolution() is incomplete!!" << std::endl;
  bool valid = true;
  int xmax = 0, ymax = 0;
  for (auto p : solution.wall) {
    xmax = std::max(xmax, p.x);
    ymax = std::max(ymax, p.y);
  }
  auto mark_invalid = [&](std::string msg) {
    std::cerr << msg << std::endl;
    valid = false;
  };

  // size.
  if (xmax > tSize) {
    mark_invalid("large W");
  }
  if (ymax > tSize) {
    mark_invalid("large H");
  }
  if (xmax < tSize / 10) {
    mark_invalid("small W");
  }
  if (ymax < tSize / 10) {
    mark_invalid("small H");
  }
  // fill walls.
  Map2D map(xmax + 1, ymax + 1, CellType::kObstacleBit);
  if (!fillPolygon(map, solution.wall, CellType::kEmpty)) {
    mark_invalid("unable to fill walls");
  }
  // area.
  const int area = countCellsByMask(map, CellType::kObstacleBit, 0);
  if (area >= (tSize * tSize * 2 + 10 - 1) / 10) {
    mark_invalid("small area");
  }
  // in/out
  for (auto p : iSqs) {
    if (!map.isInside(p) || (map(p) & CellType::kObstacleBit) != 0) {
      mark_invalid("inside point is out");
    }
  }
  for (auto p : oSqs) {
    if (!map.isInside(p) || (map(p) & CellType::kObstacleBit) == 0) {
      mark_invalid("outside point is in");
    }
  }
  // polygon vertices.
  if (solution.wall.size() < vMin) {
    mark_invalid("too few vertices");
  }
  if (solution.wall.size() > vMax) {
    mark_invalid("too many vertices");
  }
  // wrapper pos
  if (!map.isInside(solution.wrapper) || (map(solution.wrapper) & CellType::kObstacleBit) != 0) {
    mark_invalid("invalid wrapper pos");
  }
  // boosters
  if (solution.Bs.size() != mNum) { mark_invalid("booster B size mismatch"); }
  if (solution.Fs.size() != mNum) { mark_invalid("booster F size mismatch"); }
  if (solution.Ls.size() != mNum) { mark_invalid("booster L size mismatch"); }
  if (solution.Rs.size() != mNum) { mark_invalid("booster R size mismatch"); }
  if (solution.Cs.size() != mNum) { mark_invalid("booster C size mismatch"); }
  if (solution.Xs.size() != mNum) { mark_invalid("booster X size mismatch"); }
  std::vector<std::vector<Point>> boosters_list = {
    solution.Bs,
    solution.Fs,
    solution.Ls,
    solution.Rs,
    solution.Cs,
    solution.Xs,
  };
  std::vector<Point> boosters;
  for (auto& ls : boosters_list) {
    for (auto p : ls) {
      if (std::find(boosters.begin(), boosters.end(), p) != boosters.end()) {
        mark_invalid("multiple boosters at the same location");
      }
      if (!map.isInside(p) || (map(p) & CellType::kObstacleBit) != 0) {
        mark_invalid("invalid booster pos");
      }
    }
  }
  return valid;
}

Map2D Puzzle::constraintsToMap() const {
  Map2D map2d(tSize, tSize, BLANK);
  for (auto p : iSqs) { assert (map2d.isInside(p)); map2d(p) = IN; }
  for (auto p : oSqs) { assert (map2d.isInside(p)); map2d(p) = OUT; }
  return map2d;
}

std::string PuzzleSolution::toString() const {
  std::ostringstream oss;
  for (int i = 0; i < wall.size(); ++i) {
    oss << wall[i];
    if (i + 1 != wall.size()) oss << ",";
  }
  oss << "#";
  oss << wrapper;
  oss << "#";
  // no obstacles
  oss << "#";
  for (auto p : Bs) { oss << "B" << p << ";"; }
  for (auto p : Fs) { oss << "F" << p << ";"; }
  for (auto p : Ls) { oss << "L" << p << ";"; }
  for (auto p : Rs) { oss << "R" << p << ";"; }
  for (auto p : Cs) { oss << "C" << p << ";"; }
  for (auto p : Xs) { oss << "X" << p << ";"; }
  std::string s(oss.str());
  s = s.substr(0, s.size() - 1); // training ";"
  return s;
}

namespace {
void SkipChar(char*& p, char ch) {
  assert (*p == ch);
  ++p;
}

int ParseNum(char*& p) {
  int n = std::strtol(p, &p, 10);
  return n;
}

Point ParsePoint(char*& p) {
  assert (*p == '(');
  Point pos;
  int x = std::strtol(++p, &p, 10);
  assert (*p == ',');
  int y = std::strtol(++p, &p, 10);
  assert (*p == ')');
  ++p;

  return {x, y};
}

std::vector<Point> ParsePolygon(char*& p) {
  std::vector<Point> map_pos;

  assert (*p == '(');
  map_pos.emplace_back(ParsePoint(p));
  while (*p == ',') {
    map_pos.emplace_back(ParsePoint(++p));
  }

  return map_pos;
}

}  // namespace

Puzzle parsePuzzleCondString(std::string cond_file_str) {
  Puzzle res;

  char* p = const_cast<char*>(cond_file_str.data());
  res.bNum = ParseNum(p); SkipChar(p, ',');
  res.eNum = ParseNum(p); SkipChar(p, ',');
  res.tSize = ParseNum(p); SkipChar(p, ',');
  res.vMin = ParseNum(p); SkipChar(p, ',');
  res.vMax = ParseNum(p); SkipChar(p, ',');
  res.mNum = ParseNum(p); SkipChar(p, ',');
  res.fNum = ParseNum(p); SkipChar(p, ',');
  res.dNum = ParseNum(p); SkipChar(p, ',');
  res.rNum = ParseNum(p); SkipChar(p, ',');
  res.cNum = ParseNum(p); SkipChar(p, ',');
  res.xNum = ParseNum(p); SkipChar(p, '#');
  res.iSqs = ParsePolygon(p); SkipChar(p, '#');
  res.oSqs = ParsePolygon(p);

  return res;
}

std::vector<std::string> dumpPuzzleConstraintMapString(const Map2D& map2d) {
  std::vector<std::vector<char>> charmap;

  for (int y = 0; y < map2d.H; ++y) {
    std::vector<char> line(map2d.W, WALL);
    for (int x = 0; x < map2d.W; ++x) {
      char c = ' ';
      if (map2d(x, y) == BLANK) c = '.';
      if (map2d(x, y) == IN) c = '+';
      if (map2d(x, y) == OUT) c = '-';
      line[x] = c;
    }
    charmap.push_back(line);
  }

  std::reverse(charmap.begin(), charmap.end()); // now charmap[0] is the highest y.

  std::vector<std::string> result;
  for (auto& line : charmap) {
    result.push_back(std::string(line.begin(), line.end()));
  }
  return result;

}

std::ostream& operator<<(std::ostream& os, const Puzzle& puzzle) {
  os << "Puzzle("
     << "bNum=" << puzzle.bNum << ","
     << "eNum=" << puzzle.eNum  << ","
     << "tSize=" << puzzle.tSize  << ","
     << "vMin=" << puzzle.vMin  << ","
     << "vMax=" << puzzle.vMax  << ","
     << "\n       "
     << "M=" << puzzle.mNum  << ","
     << "F=" << puzzle.fNum  << ","
     << "D=" << puzzle.dNum  << ","
     << "R=" << puzzle.rNum  << ","
     << "C=" << puzzle.cNum  << ","
     << "X=" << puzzle.xNum  << ","
     << "\n       iSqs[" << puzzle.iSqs.size() << "]=";
  for (auto p : puzzle.iSqs) {
    os << p;
  }
  os << ","
     << "\n       oSqs[" << puzzle.oSqs.size() << "]=";
  for (auto p : puzzle.oSqs) {
    os << p;
  }
  os << ")";
  return os;
}