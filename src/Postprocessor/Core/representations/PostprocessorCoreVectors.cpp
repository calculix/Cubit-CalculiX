#include "../application/PostprocessorCore.hpp"
#include "ResultVectors.hpp"
#include <limits>
namespace Postprocessor
{
  bool PostprocessorCore::set_vector_plot(const std::string& source_id,const std::string& view_id,
    const VectorPlotDefinition& value,std::string& error)
  {
    if (!valid_vector_plot(value,error))
    {
      return false;
    }
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot change vectors during the current operation.";
      return false;
    }
    for (const auto& entry : state.representations)
    {
      const auto& display = entry.second;
      if (!display_matches_source(display,source_id,view_id))
      {
        continue;
      }
      if (same_vector_plot(display.vectors,value))
      {
        return true;
      }
      if ((value.enabled && !display.vectors.enabled) || !same_field(display.vectors.field,value.field))
      {
        std::shared_ptr<const EvaluatedOutput> evaluated;
        if (!evaluate_output(display,evaluated,error))
        {
          return false;
        }
        bool found = false;
        if (value.field && display.increment_index < evaluated->data->increments.size())
        {
          const auto& frame = evaluated->data->increments[display.increment_index];
          for (const auto& field : frame.fields)
          {
            auto selection = *value.field;
            if (selection.dataset_id && *selection.dataset_id != field.dataset_id)
            {
              continue;
            }
            selection.dataset_id = field.dataset_id;
            found = found || (find_field(frame,selection) == &field && spatial_vector(field));
          }
        }
        if (!found)
        {
          error = "The selected output has no matching spatial vector in the current frame.";
          return false;
        }
      }
      auto staged = state;
      staged.representations.at(entry.first).vectors = value;
      state.swap(staged);
      ++scene_revision;
      return true;
    }
    error = "The source has no representation in the target view.";
    return false;
  }
}
