#ifndef CORETOPOLOGYTRAJECTORYPRESERVATION_HPP
#define CORETOPOLOGYTRAJECTORYPRESERVATION_HPP

#include <set>

class CoreLoadsTrajectory;
class CoreBlocks;
class MeshExportInterface;

class CoreTopologyTrajectoryPreservation
{
public:
  static bool append(CoreLoadsTrajectory* trajectories, CoreBlocks* blocks, MeshExportInterface& mesh, const std::set<int>& trajectory_ids, std::set<int>& elements);
};

#endif
