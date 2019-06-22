#pragma once
#include <vector>
#include "base.h"
#include "map2d.h"

struct Puzzle {
  int bNum = 0; // block
  int eNum = 0; // epoch
  // map geometry
  int tSize = 0; // maximum of the coordinates, and used for other validation.
  int vMin = 0; // # of vertices
  int vMax = 0; // # of vertices
  // # of boosters
  int mNum = 0;
  int fNum = 0;
  int dNum = 0;
  int rNum = 0;
  int cNum = 0;
  int xNum = 0;
  std::vector<Point> iSqs; // include these
  std::vector<Point> oSqs; // exclude these

  bool operator!=(const Puzzle& rhs) const {
    return !operator==(rhs);
  }
  bool operator==(const Puzzle& rhs) const {
    return
      bNum == rhs.bNum &&
      eNum == rhs.eNum &&
      tSize == rhs.tSize &&
      vMin == rhs.vMin &&
      vMax == rhs.vMax &&
      mNum == rhs.mNum &&
      fNum == rhs.fNum &&
      dNum == rhs.dNum &&
      rNum == rhs.rNum &&
      cNum == rhs.cNum &&
      xNum == rhs.xNum &&
      iSqs == rhs.iSqs &&
      oSqs == rhs.oSqs;
  }
  bool validateSolution(const Map2D& solution_map) const;
  Map2D constraintsToMap() const;
};

struct PuzzleSolution {
  Polygon wall;
  Point wrapper;
  std::vector<Point> Bs;
  std::vector<Point> Fs;
  std::vector<Point> Ls;
  std::vector<Point> Rs;
  std::vector<Point> Cs;
  std::vector<Point> Xs;

  std::string toString() const;
};

Puzzle parsePuzzleCondString(std::string cond_file_str);

// dump constraints map for display. the first row corresponds to the highest y.
// do not contain line terminator at the end of lines.
std::vector<std::string> dumpPuzzleConstraintMapString(const Map2D& map2d);

std::ostream& operator<<(std::ostream&, const Puzzle&);
