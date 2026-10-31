#include "CoreTopologyAdjointOutputs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "CubitMessage.hpp"
#include <cstddef>
#include <string>
#include <vector>

namespace
{
template <typename Outputs>
bool append_outputs(Outputs& outputs, const std::vector<std::vector<int>>& assignments, const std::vector<int>& indices, const char* kind, const char* label, std::ostream& output)
{
  for (int index : indices)
  {
    if (index < 0 || static_cast<std::size_t>(index) >= assignments.size() || assignments[index].size() < 2)
    {
      PRINT_ERROR("Invalid %s output assignment for the final analysis step.\n", kind);
      return false;
    }

    const int output_id = assignments[index][1];

    if (outputs.get_outputs_data_id_from_output_id(output_id) < 0)
    {
      PRINT_ERROR("%s output %d no longer exists.\n", label, output_id);
      return false;
    }

    const std::string data = outputs.get_output_export(output_id);

    if (data.empty())
    {
      PRINT_ERROR("Unable to export %s output %d.\n", kind, output_id);
      return false;
    }

    output << data;

    if (data.back() != '\n')
    {
      output << "\n";
    }
  }

  return true;
}
}

bool CoreTopologyAdjointOutputs::append_history(CoreSteps& steps, CoreHistoryOutputs& outputs, int group_id, std::ostream& output)
{
  const auto indices = steps.get_historyoutput_data_ids_from_historyoutputs_id(group_id);
  return append_outputs(outputs, steps.historyoutputs_data, indices, "history", "History", output);
}

bool CoreTopologyAdjointOutputs::append_field(CoreSteps& steps, CoreFieldOutputs& outputs, int group_id, std::ostream& output)
{
  const auto indices = steps.get_fieldoutput_data_ids_from_fieldoutputs_id(group_id);
  return append_outputs(outputs, steps.fieldoutputs_data, indices, "field", "Field", output);
}
