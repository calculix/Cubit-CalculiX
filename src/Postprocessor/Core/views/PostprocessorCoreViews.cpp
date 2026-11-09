#include "../application/PostprocessorCore.hpp"
#include "../representations/ResultMesh.hpp"
#include "CameraOperations.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>

namespace Postprocessor
{
  bool PostprocessorCore::visible_bounds(const std::string& view_id, std::array<double, 6>& bounds, std::string& error) const
  {
    error.clear();
    if (state.views.find(view_id) == state.views.end())
    {
      error = "The target view is not registered.";
      return false;
    }
    try
    {
      bool found = false;
      std::array<double, 6> combined = {};
      for (const auto& entry : state.representations)
      {
        const auto& display = entry.second;
        if (!display.visible || display.view_id != view_id || display.input.owner != OutputOwner::Source || display.input.port != 0)
        {
          continue;
        }
        const auto data = result_data(display.input.object_id);
        if (!data || display.increment_index >= data->increments.size())
        {
          continue;
        }
        const auto& frame = data->increments[display.increment_index];
        for (const auto& part : result_mesh_parts(frame.dataset.get()))
        {
          if (!dataset_visible(display,part.first))
          {
            continue;
          }
          std::shared_ptr<vtkDataSet> transformed;
          std::string warning;
          if (!deform_mesh(part.second,part.first,frame,display.deformation,transformed,warning,error))
          {
            return false;
          }
          std::array<double, 6> current;
          if (!mesh_bounds(transformed ? transformed.get() : part.second,current))
          {
            continue;
          }
          if (display.deformation.enabled && display.deformation.overlay)
          {
            std::array<double,6> original;
            if (mesh_bounds(part.second,original))
            {
              for (std::size_t axis = 0; axis < 3; ++axis)
              {
                current[axis*2] = std::min(current[axis*2],original[axis*2]);
                current[axis*2+1] = std::max(current[axis*2+1],original[axis*2+1]);
              }
            }
          }
          if (!found)
          {
            combined = current;
            found = true;
          }
          else
          {
            for (std::size_t axis = 0; axis < 3; ++axis)
            {
              combined[axis * 2] = std::min(combined[axis * 2], current[axis * 2]);
              combined[axis * 2 + 1] = std::max(combined[axis * 2 + 1], current[axis * 2 + 1]);
            }
          }
        }
      }
      if (!found)
      {
        // Empty is a normal query result: leave bounds untouched and error empty.
        return false;
      }
      bounds = combined;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot compute visible mesh bounds: ") + exception.what();
      return false;
    }
  }

  bool PostprocessorCore::request_camera_fit(const std::string& view_id, std::string& error)
  {
    error.clear();
    const auto found = state.views.find(view_id);
    if (source_operation_active || found == state.views.end() || (found->second.fit_request == std::numeric_limits<std::uint64_t>::max() ||
        found->second.camera_revision == std::numeric_limits<std::uint64_t>::max()))
    {
      error = "Cannot request a fit for this view during the current operation.";
      return false;
    }
    std::array<double, 6> bounds;
    if (!visible_bounds(view_id, bounds, error))
    {
      // A valid empty view accepts Fit as a no-op without advancing its tokens.
      return error.empty();
    }
    found->second.fit_pending = true;
    ++found->second.fit_request;
    ++found->second.camera_revision;
    return true;
  }

  bool PostprocessorCore::complete_camera_fit(const std::string& view_id, std::uint64_t request,
    std::uint64_t revision, const CameraDefinition& camera, std::string& error)
  {
    error.clear();
    const auto found = state.views.find(view_id);
    if (source_operation_active || found == state.views.end() || !found->second.fit_pending ||
        found->second.fit_request != request || scene_revision != revision ||
        found->second.camera_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "The camera fit request is no longer current.";
      return false;
    }
    if (!CameraOperations::valid(camera))
    {
      error = "The fitted camera is invalid.";
      return false;
    }
    found->second.camera = camera;
    found->second.fit_pending = false;
    ++found->second.camera_revision;
    return true;
  }
}
