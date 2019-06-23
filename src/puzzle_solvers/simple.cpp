#include <memory>
#include <string>

#include "puzzle.h"
#include "fill_polygon.h"
#include "solver_registry.h"

PuzzleSolution simpleSolver(PuzzleSolverParam param, Puzzle puzzle) {
  // just returns a fixed (invalid) solution.
  constexpr int R = 1;
  Map2D map2d(3, 3, {
    0, 0, R,
    0, R, R,
    0, 0, R,
  });
  Polygon fine_polygon;
  assert (parsePolygon(fine_polygon, map2d, R));
  
  PuzzleSolution solution;
  solution.wall = simplifyPolygon(fine_polygon);
  return solution;
}

REGISTER_PUZZLE_SOLVER("simple", simpleSolver);
