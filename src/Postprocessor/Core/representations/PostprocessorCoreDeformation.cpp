#include "../application/PostprocessorCore.hpp"
#include <exception>
#include <limits>

namespace Postprocessor
{
  bool PostprocessorCore::set_deformation(const std::string& source_id, const std::string& view_id,
    const ResultDeformation& value, std::string& error)
  {
    error.clear();
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        !valid_deformation(value,error))
    {
      if (error.empty())
      {
        error = "Cannot change deformation during the current operation.";
      }
      return false;
    }
    try
    {
      for (const auto& entry : state.representations)
      {
        const auto& display = entry.second;
        if (display.input.owner != OutputOwner::Source || display.input.object_id != source_id ||
            display.input.port != 0 || display.view_id != view_id)
        {
          continue;
        }
        if (same_deformation(display.deformation,value))
        {
          return true;
        }
        // Enabling or selecting another field must resolve now. Frame changes
        // may subsequently lack that field; the rendering helper handles fallback.
        const bool changedSelection = value.field_mode != display.deformation.field_mode ||
          (value.field_mode == DeformationFieldMode::Explicit && !same_field(value.field,display.deformation.field));
        if ((changedSelection && value.field_mode == DeformationFieldMode::Explicit) ||
            (value.enabled && (changedSelection || !display.deformation.enabled)))
        {
          const auto data = result_data(source_id);
          if (!data || display.increment_index >= data->increments.size() ||
              !deformation_available(data->increments[display.increment_index],value,error))
          {
            if (error.empty())
            {
              error = "Deformation requires an available point-associated displacement vector.";
            }
            return false;
          }
        }
        auto staged = state;
        staged.representations.at(entry.first).deformation = value;
        state.swap(staged);
        ++scene_revision;
        return true;
      }
      error = "The source has no representation in the target view.";
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot change deformation: ")+exception.what();
    }
    catch (...)
    {
      error = "Cannot change deformation: unknown failure.";
    }
    return false;
  }
}
