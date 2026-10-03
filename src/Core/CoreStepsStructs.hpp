#ifndef CORESTEPSSTRUCTS_HPP
#define CORESTEPSSTRUCTS_HPP

#include <vector>
#include <string>

struct StepSplitPiece
{
  int step_id;
  double begin;
  double end;

  // -1 denotes waiting/cooling outside the trajectory.
  int trajectory_interval;
};

struct StepSplitResult
{
  bool valid = false;
  std::vector<StepSplitPiece> pieces;
};

struct PlannedPiece
{
  double begin;
  double end;
  int trajectory_interval;
};

#endif // CORESTEPSSTRUCTS_HPP