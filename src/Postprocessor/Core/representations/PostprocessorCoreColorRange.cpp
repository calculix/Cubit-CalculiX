#include "../application/PostprocessorCore.hpp"
#include "Postprocessor/Core/sources/SourceRuntime.hpp"
#include "ResultMesh.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <exception>
#include <algorithm>
namespace Postprocessor
{
  std::optional<std::array<double,2>> PostprocessorCore::all_frame_range(const std::string& source_id,
    const FieldSelection& selection,std::string& error,const std::set<std::size_t>& hidden) const
  {
    error.clear();
    const auto* loaded = source_runtime(source_id);
    if (!loaded || !loaded->data) {
      error = "The result source is unavailable.";
      return std::nullopt;
    }
    for (const auto& group : loaded->groups)
    {
      if (!group.unit_issue.empty() && (group.key == selection.group_key ||
          (selection.group_key.empty() && group.field.array_name == selection.array_name && group.field.association == selection.association)))
      {
        error = group.unit_issue;
        return std::nullopt;
      }
    }
    try {
      if (loaded->unit_generation != framework_units().generation()) {
        loaded->all_frame_ranges.clear();
        loaded->unit_generation = framework_units().generation();
      }
      std::optional<std::array<double,2>> range;
      for (const auto& dataset : loaded->datasets) {
        if (hidden.count(dataset.id) != 0 || (selection.dataset_id && *selection.dataset_id != dataset.id)) {
          continue;
        }
        auto selected = selection;
        selected.dataset_id = dataset.id;
        const auto key = field_selection_key(selected);
        auto found = loaded->all_frame_ranges.find(key);
        if (found == loaded->all_frame_ranges.end()) {
          std::optional<std::array<double,2>> partRange;
          for (const auto& increment : loaded->data->increments) {
            const auto* field = find_field(increment,selected);
            if (!field) {
              continue;
            }
            for (const auto& part : result_mesh_parts(increment.dataset.get())) {
              if (part.first == dataset.id && !accumulate_color_range(part.second,*field,selected,partRange,error,captured_units())) {
                return std::nullopt;
              }
            }
          }
          found = loaded->all_frame_ranges.emplace(key,partRange).first;
        }
        if (found->second) {
          if (!range) {
            range = found->second;
          }
          else {
            (*range)[0] = std::min((*range)[0],(*found->second)[0]);
            (*range)[1] = std::max((*range)[1],(*found->second)[1]);
          }
        }
      }
      return range;
    }
    catch (const std::exception& exception) {
      error = std::string("Cannot compute all-frame color range: ")+exception.what();
    }
    catch (...) {
      error = "Cannot compute all-frame color range: unknown failure.";
    }
    return std::nullopt;
  }
}
