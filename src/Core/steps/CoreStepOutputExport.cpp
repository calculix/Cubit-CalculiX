#include "CoreStepOutputExport.hpp"
#include "CoreSteps.hpp"
#include "CoreHistoryOutputs.hpp"
#include "CoreFieldOutputs.hpp"

namespace
{
  template<typename OutputType>
  void append_outputs(
    OutputType& outputs, const std::vector<std::vector<int>>& references,
    const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
  {
    for (int assignment_id : assignment_ids)
    {
      const int output_id = references[assignment_id][1];
      lines.push_back(outputs.get_output_export(output_id));
    }
  }
}

void CoreStepOutputExport::append_history(
  CoreSteps& steps, CoreHistoryOutputs& outputs,
  int historyoutputs_id, std::vector<std::string>& lines)
{
  const auto ids = steps.get_historyoutput_data_ids_from_historyoutputs_id(historyoutputs_id);
  append_outputs(outputs, steps.historyoutputs_data, ids, lines);
}

void CoreStepOutputExport::append_field(
  CoreSteps& steps, CoreFieldOutputs& outputs,
  int fieldoutputs_id, std::vector<std::string>& lines)
{
  const auto ids = steps.get_fieldoutput_data_ids_from_fieldoutputs_id(fieldoutputs_id);
  append_outputs(outputs, steps.fieldoutputs_data, ids, lines);
}
