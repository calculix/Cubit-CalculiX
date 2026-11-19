#include "PostprocessorCore.hpp"
#include "Postprocessor/Core/sources/SourceProjectRuntime.hpp"
#include "../representations/ResultColoringUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <limits>
#include <utility>

bool Postprocessor::PostprocessorCore::configure_units(const std::string& system, const std::string& quantity, const std::string& unit, std::string& error)
{
  error.clear();
  if (captureUnits)
  {
    error = "The capture presentation context is immutable.";
    return false;
  }
  if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() || state.plot_revision == std::numeric_limits<std::uint64_t>::max())
  {
    error = "Cannot change units during a source operation or after runtime identifiers are exhausted.";
    return false;
  }
  auto stagedUnits = framework_units();
  bool accepted = false;
  if (!system.empty())
  {
    accepted = stagedUnits.set_workspace_system("postprocessor", system, error);
  }
  else
  {
    accepted = stagedUnits.set_display_unit("postprocessor", quantity, unit, error);
  }
  if (!accepted)
  {
    return false;
  }
  auto candidate = state;
  for (auto& entry : candidate.representations)
  {
    auto vectorLegend = entry.second.vectors.legend;
    vectorLegend.field = entry.second.vectors.field;
    if (vectorLegend.field && !vectorLegend.bounds_unit)
    {
      const auto source = result_data(output_source(entry.second.input));
      if (source)
      {
        for (const auto& frame : source->increments)
        {
          const auto* field = find_field(frame,*vectorLegend.field);
          if (field)
          {
            if (!bind_coloring_units(vectorLegend,*field,error))
            {
              return false;
            }
            break;
          }
        }
      }
    }
    if (!convert_coloring_units(vectorLegend,stagedUnits,error))
    {
      return false;
    }
    vectorLegend.field.reset();
    entry.second.vectors.legend = std::move(vectorLegend);
    auto& coloring = entry.second.coloring;
    if (coloring.field && !coloring.bounds_unit)
    {
      // Legacy settings must bind in the OLD presentation context. Search all
      // frames because the current frame need not contain the selected field.
      const auto source = result_data(output_source(entry.second.input));
      if (source)
      {
        for (const auto& frame : source->increments)
        {
          const auto* field = find_field(frame, *coloring.field);
          if (field)
          {
            if (!bind_coloring_units(coloring, *field, error))
            {
              return false;
            }
            break;
          }
        }
      }
    }
    if (!convert_coloring_units(coloring, stagedUnits, error))
    {
      return false;
    }
  }
  for (auto& entry : candidate.filters)
  {
    auto& filter = entry.second;
    if (filter.kind == Postprocessor::FilterKind::Threshold)
    {
      if (filter.revision == std::numeric_limits<std::uint64_t>::max())
      {
        error = "Threshold revision identifiers are exhausted.";
        return false;
      }
      if (!Postprocessor::convert_threshold_units(filter.threshold,stagedUnits,error))
      {
        return false;
      }
      ++filter.revision;
    }
  }
  ++candidate.plot_revision;
  for (auto& entry : candidate.plots)
  {
    if (!entry.second.axes.x_unit)
    {
      entry.second.axes.x.reset();
    }
    if (!entry.second.axes.y_unit)
    {
      entry.second.axes.y.reset();
    }
    entry.second.data_revision = candidate.plot_revision;
  }
  // Both publications are non-throwing; no global context is changed on failure.
  framework_units().swap_project_state(stagedUnits);
  state.swap(candidate);
  invalidate_filter_runtime();
  invalidate_explosion_runtime();
  sourceProjectRuntime->invalidate_color_ranges();
  ++scene_revision;
  return true;
}
