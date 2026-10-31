#ifndef CORETOPOLOGYHISTORYOUTPUTASSIGNMENT_HPP
#define CORETOPOLOGYHISTORYOUTPUTASSIGNMENT_HPP

#include <functional>
#include <string>
#include <vector>

class CoreSteps;

class CoreTopologyHistoryOutputAssignment
{
public:
  static bool collect_ids(CoreSteps& steps, int step_id, std::vector<int>& assigned_ids);
  static bool ensure_assigned(CoreSteps& steps, int step_id, int output_id, const std::vector<int>& assigned_ids, const std::function<bool(const std::string&)>& command, bool& needs_assignment);
};

#endif
