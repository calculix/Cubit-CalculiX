#ifndef CORETOPOLOGYNODEHISTORYOUTPUTPREPARATION_HPP
#define CORETOPOLOGYNODEHISTORYOUTPUTPREPARATION_HPP

#include <functional>
#include <string>

class CoreHistoryOutputs;
class CoreSteps;

class CoreTopologyNodeHistoryOutputPreparation
{
public:
  static bool ensure(CoreHistoryOutputs& outputs, CoreSteps& steps, int topology_id, int step_id, int nodeset_id, const std::string& key, const std::function<bool(const std::string&)>& execute_command);
};

#endif
