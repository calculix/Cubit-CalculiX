#include "CoreStepModelChange.hpp"
#include "CoreSteps.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"

#include <algorithm>
#include <cstddef>
#include <string>

bool CoreStepModelChange::create_dummy_step(
  CoreSteps& steps, CoreCustomLines& customlines, CoreLoadsTrajectory& trajectory,
  int step_id, const std::vector<int>& trajectory_ids)
{
  if (!steps.create_modelchangeelement_dummystep(step_id))
  {
    return false;
  }

  std::vector<int> element_ids;

  for (int trajectory_id : trajectory_ids)
  {
    const auto ids = trajectory.get_bodyheatfluxsphere_draw_element_ids(trajectory_id);

    if (ids.empty())
    {
      return false;
    }

    for (const auto& node : ids)
    {
      for (const auto& radius : node)
      {
        element_ids.insert(element_ids.end(), radius.begin(), radius.end());
      }
    }
  }

  std::sort(element_ids.begin(), element_ids.end());
  element_ids.erase(std::unique(element_ids.begin(), element_ids.end()), element_ids.end());

  std::string line = "*MODEL CHANGE,TYPE=ELEMENT,REMOVE\n";
  std::size_t column = 0;

  for (int element_id : element_ids)
  {
    line += std::to_string(element_id);

    if (++column == 16)
    {
      line += ",\n";
      column = 0;
    }
    else
    {
      line += ",";
    }
  }

  return customlines.create_customline({"DUMMY STEP 2", "AFTER", "STEP_BEGIN", "2", line});
}
