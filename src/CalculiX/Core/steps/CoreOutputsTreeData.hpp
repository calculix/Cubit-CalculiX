#ifndef COREOUTPUTSTREEDATA_HPP
#define COREOUTPUTSTREEDATA_HPP

#include <string>
#include <vector>

class CoreHistoryOutputs;
class CoreFieldOutputs;
class CoreSteps;

class CoreOutputsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_historyoutputs_tree_data(CoreHistoryOutputs& outputs);
  static std::vector<std::vector<std::string>> get_fieldoutputs_tree_data(CoreFieldOutputs& outputs);

  static std::vector<std::vector<std::string>> get_steps_historyoutputs_tree_data(
    CoreSteps& steps, CoreHistoryOutputs& outputs, int step_id);

  static std::vector<std::vector<std::string>> get_steps_fieldoutputs_tree_data(
    CoreSteps& steps, CoreFieldOutputs& outputs, int step_id);
};

#endif
