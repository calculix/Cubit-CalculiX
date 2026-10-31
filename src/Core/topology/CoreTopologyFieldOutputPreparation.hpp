#ifndef CORETOPOLOGYFIELDOUTPUTPREPARATION_HPP
#define CORETOPOLOGYFIELDOUTPUTPREPARATION_HPP

#include <functional>
#include <string>

class CoreFieldOutputs;
class CoreSteps;

class CoreTopologyFieldOutputPreparation
{
public:
  static bool ensure_displacement(CoreFieldOutputs& outputs, CoreSteps& steps, int topology_id, int step_id, const std::function<bool(const std::string&)>& command);
};

#endif
