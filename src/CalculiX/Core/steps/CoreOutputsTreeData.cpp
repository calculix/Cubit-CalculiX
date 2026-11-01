#include "CoreOutputsTreeData.hpp"
#include "CoreHistoryOutputs.hpp"
#include "CoreFieldOutputs.hpp"
#include "CoreSteps.hpp"
#include "CubitMessage.hpp"

namespace
{
  template<typename OutputType>
  std::vector<std::string> get_output_tree_row(
    OutputType& outputs, const std::vector<int>& output, bool include_section)
  {
    const int name_data_id = outputs.get_name_data_id_from_name_id(output[1]);

    if (name_data_id < 0)
    {
      PRINT_ERROR("Name data missing for Output ID %d.\n", output[0]);
      return {};
    }

    std::string name = outputs.name_data[name_data_id][1];

    if (output[2] == 1)
    {
      name += " (node)";
    }
    else if (output[2] == 2)
    {
      name += " (element)";
    }
    else if (output[2] == 3)
    {
      name += " (contact)";
    }
    else if (include_section && output[2] == 4)
    {
      name += " (section)";
    }

    return {std::to_string(output[0]), name};
  }

  template<typename OutputType>
  std::vector<std::vector<std::string>> get_output_tree_data(OutputType& outputs, bool include_section)
  {
    std::vector<std::vector<std::string>> tree_data;

    for (const auto& output : outputs.outputs_data)
    {
      const auto row = get_output_tree_row(outputs, output, include_section);

      if (!row.empty())
      {
        tree_data.push_back(row);
      }
    }

    return tree_data;
  }

  template<typename OutputType>
  std::vector<std::vector<std::string>> get_step_output_tree_data(
    OutputType& outputs, const std::vector<std::vector<int>>& references,
    const std::vector<int>& data_ids, bool include_section)
  {
    std::vector<std::vector<std::string>> tree_data;

    for (int data_id : data_ids)
    {
      const int output_id = references[data_id][1];
      const int output_data_id = outputs.get_outputs_data_id_from_output_id(output_id);

      if (output_data_id < 0)
      {
        PRINT_ERROR("Step references missing Output ID %d.\n", output_id);
        continue;
      }

      const auto row = get_output_tree_row(outputs, outputs.outputs_data[output_data_id], include_section);

      if (!row.empty())
      {
        tree_data.push_back(row);
      }
    }

    return tree_data;
  }
}

std::vector<std::vector<std::string>> CoreOutputsTreeData::get_historyoutputs_tree_data(CoreHistoryOutputs& outputs)
{
  return get_output_tree_data(outputs, true);
}

std::vector<std::vector<std::string>> CoreOutputsTreeData::get_fieldoutputs_tree_data(CoreFieldOutputs& outputs)
{
  return get_output_tree_data(outputs, false);
}

std::vector<std::vector<std::string>> CoreOutputsTreeData::get_steps_historyoutputs_tree_data(
  CoreSteps& steps, CoreHistoryOutputs& outputs, int step_id)
{
  int step_data_id = steps.get_steps_data_id_from_step_id(step_id);

  if (step_data_id == -1)
  {
    return {};
  }

  std::vector<int> data_ids = steps.get_historyoutput_data_ids_from_historyoutputs_id(steps.steps_data[step_data_id][7]);
  return get_step_output_tree_data(outputs, steps.historyoutputs_data, data_ids, true);
}

std::vector<std::vector<std::string>> CoreOutputsTreeData::get_steps_fieldoutputs_tree_data(
  CoreSteps& steps, CoreFieldOutputs& outputs, int step_id)
{
  int step_data_id = steps.get_steps_data_id_from_step_id(step_id);

  if (step_data_id == -1)
  {
    return {};
  }

  std::vector<int> data_ids = steps.get_fieldoutput_data_ids_from_fieldoutputs_id(steps.steps_data[step_data_id][8]);
  return get_step_output_tree_data(outputs, steps.fieldoutputs_data, data_ids, false);
}
