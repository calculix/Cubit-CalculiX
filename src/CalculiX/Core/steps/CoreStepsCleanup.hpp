#ifndef CORESTEPSCLEANUP_HPP
#define CORESTEPSCLEANUP_HPP

#include <functional>
#include <string>

class CoreSteps;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreHistoryOutputs;
class CoreFieldOutputs;

class CoreStepsCleanup
{
public:
  static bool cleanup_loads(CoreSteps& steps, int step_id, int loads_id, const std::function<bool(int, int)>& load_exists, std::string& log);

  static bool cleanup_bcs(CoreSteps& steps, int step_id, int bcs_id, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, std::string& log);
  static bool cleanup_outputs(CoreSteps& steps, int step_id, int history_id, int field_id, CoreHistoryOutputs& history, CoreFieldOutputs& field, std::string& log);

  static bool cleanup(CoreSteps& steps, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, CoreHistoryOutputs& history, CoreFieldOutputs& field, const std::function<bool(int, int)>& load_exists, std::string& log);
};

#endif
