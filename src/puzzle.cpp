#include "puzzle.h"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <cctype>

bool Puzzle::validateSolution(const Map2D& solution_map) {
  assert (false);
  return false;
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