#include "../application/PostprocessorCore.hpp"
#include "ResultVectors.hpp"
#include "ResultColoringUnits.hpp"
#include <limits>
namespace Postprocessor
{
  bool PostprocessorCore::set_vector_plot(const std::string& source_id,const std::string& view_id,
    const VectorPlotDefinition& value,std::string& error)
  {
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
      auto candidate = value;
      if (candidate.enabled && !candidate.field)
      {
        const auto groups = field_groups(source_id,std::nullopt);
        for (const auto& group : groups)
        {
          ResultFieldSelector selector;
          if (vector_field_choice(groups,group,selector))
          {
            candidate.field = preferred_field_selection(group.field,group.key);
            break;
          }
        }
        if (!candidate.field)
        {
          error = "The source has no eligible vector fields.";
          return false;
        }
      }
      if (!valid_vector_plot(candidate,error))
      {
        return false;
      }
      if (same_vector_plot(display.vectors,candidate))
      {
        return true;
      }
      if (candidate.field && ((candidate.enabled && !display.vectors.enabled) || !same_field(display.vectors.field,candidate.field)))
      {
        std::shared_ptr<const EvaluatedOutput> evaluated;
        if (!evaluate_output(display,evaluated,error))
        {
          return false;
        }
        bool found = false;
        if (candidate.field && display.increment_index < evaluated->data->increments.size())
        {
          const auto& frame = evaluated->data->increments[display.increment_index];
          for (const auto& field : frame.fields)
          {
            auto selection = *candidate.field;
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
      if (candidate.field)
      {
        const auto source = result_data(source_id);
        if (source)
        {
          for (const auto& frame : source->increments)
          {
            const auto* field = find_field(frame,*candidate.field);
            if (field)
            {
              auto bound = candidate.legend;
              bound.field = candidate.field;
              if (!bind_coloring_units(bound,*field,error))
              {
                return false;
              }
              candidate.legend.bounds_unit = bound.bounds_unit;
              break;
            }
          }
        }
      }
      auto staged = state;
      staged.representations.at(entry.first).vectors = candidate;
      state.swap(staged);
      ++scene_revision;
      return true;
    }
    error = "The source has no representation in the target view.";
    return false;
  }
}
