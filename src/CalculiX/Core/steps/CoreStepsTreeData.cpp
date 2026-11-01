#include "CoreStepsTreeData.hpp"
#include "CoreSteps.hpp"
#include "CubitMessage.hpp"

#include <array>

namespace
{
  const std::array<const char*, 11> step_type_suffixes = {
    " (NO ANALYSIS)",
    " (STATIC)",
    " (FREQUENCY)",
    " (BUCKLE)",
    " (HEAT TRANSFER)",
    " (COUPLED TEMPERATURE-DISPLACEMENT)",
    " (UNCOUPLED TEMPERATURE-DISPLACEMENT)",
    " (DYNAMIC)",
    " (MODAL DYNAMIC)",
    " (STEADY STATE DYNAMICS)",
    " (COMPLEX DYNAMIC)"
  };
}

std::vector<std::vector<std::string>> CoreStepsTreeData::get_steps_tree_data(CoreSteps& steps)
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& step : steps.steps_data)
  {
    const int name_data_id = steps.get_name_data_id_from_name_id(step[1]);

    if (name_data_id < 0)
    {
      PRINT_ERROR("Name data missing for Step ID %d.\n", step[0]);
      continue;
    }

    std::string name = steps.name_data[name_data_id][1];
    const int type = step[3];

    if (type >= 1 && type <= static_cast<int>(step_type_suffixes.size()))
    {
      name += step_type_suffixes[type - 1];
    }

    tree_data.push_back({std::to_string(step[0]), name});
  }

  return tree_data;
}
