#ifndef CORETOPOLOGYELEMENTHISTORYOUTPUTPREPARATION_HPP
#define CORETOPOLOGYELEMENTHISTORYOUTPUTPREPARATION_HPP

#include <functional>
#include <string>

class CoreHistoryOutputs;
class CoreSteps;

class CoreTopologyElementHistoryOutputPreparation
{
public:
  static bool ensure(CoreHistoryOutputs& outputs, CoreSteps& steps, int topology_id, int step_id, int block_id, const std::string& key, const std::function<bool(const std::string&)>& execute_command);
};

#endif
