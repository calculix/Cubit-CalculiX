#ifndef CORETOPOLOGYADJOINTOUTPUTS_HPP
#define CORETOPOLOGYADJOINTOUTPUTS_HPP

#include <ostream>

class CoreSteps;
class CoreHistoryOutputs;
class CoreFieldOutputs;

class CoreTopologyAdjointOutputs
{
public:
  static bool append_history(CoreSteps& steps, CoreHistoryOutputs& outputs, int group_id, std::ostream& output);
  static bool append_field(CoreSteps& steps, CoreFieldOutputs& outputs, int group_id, std::ostream& output);
};

#endif
