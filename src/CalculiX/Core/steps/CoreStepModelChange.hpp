#ifndef CORESTEPMODELCHANGE_HPP
#define CORESTEPMODELCHANGE_HPP

#include <vector>

class CoreSteps;
class CoreCustomLines;
class CoreLoadsTrajectory;

class CoreStepModelChange
{
public:
  static bool create_dummy_step(CoreSteps& steps, CoreCustomLines& customlines, CoreLoadsTrajectory& trajectory, int step_id, const std::vector<int>& trajectory_ids);
};

#endif
