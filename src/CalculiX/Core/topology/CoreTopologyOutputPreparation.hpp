#ifndef CORETOPOLOGYOUTPUTPREPARATION_HPP
#define CORETOPOLOGYOUTPUTPREPARATION_HPP

#include <functional>
#include <set>
#include <string>

class CoreTopology;
class CoreSteps;
class CoreHistoryOutputs;
class CoreFieldOutputs;
class CoreBlocks;
class MeshExportInterface;

struct CoreTopologyOutputPreparationData
{
  const CoreTopology& topology;
  CoreSteps& steps;
  CoreHistoryOutputs& historyoutputs;
  CoreBlocks* blocks;
  MeshExportInterface* mesh;
  CoreFieldOutputs* fieldoutputs;
};

class CoreTopologyOutputPreparation
{
public:
  static bool prepare(const CoreTopologyOutputPreparationData& data, int topology_id, const std::set<int>& block_ids, const std::function<bool(const std::string&)>& execute_command);
};

#endif
