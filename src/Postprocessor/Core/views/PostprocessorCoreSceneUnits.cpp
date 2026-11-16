#include "../application/PostprocessorCore.hpp"
#include "CameraOperations.hpp"
#include "../representations/DisplayDefaults.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <cmath>
#include <limits>
namespace Postprocessor
{
  double scale_interval(double available)
  {
    if (!std::isfinite(available) || available <= 0)
    {
      return 0;
    }
    const double power = std::pow(10.0,std::floor(std::log10(available)));
    if (!std::isfinite(power) || power <= 0)
    {
      return 0;
    }
    const double normalized = available/power;
    double multiple = 1;
    if (normalized >= 5)
    {
      multiple = 5;
    }
    else if (normalized >= 2)
    {
      multiple = 2;
    }
    return multiple*power;
  }
  bool PostprocessorCore::scene_scale(const Representation& display, double& factor, bool& known, std::string& error) const
  {
    error.clear();
    factor = 1;
    known = false;
    const auto view = state.views.find(display.view_id);
    const auto source = result_data(output_source(display.input));
    if (view == state.views.end())
    {
      error = "The scene view is unavailable.";
      return false;
    }
    if (!source)
    {
      return true;
    }
    const auto native = filter_coordinate_unit(*source,display.increment_index);
    if (native.empty() || view->second.scene_unit.empty())
    {
      return true;
    }
    if (!framework_units().convert(1,native,view->second.scene_unit,factor,error) || !std::isfinite(factor) || factor <= 0)
    {
      error = "Cannot transform source coordinates into the fixed scene length unit.";
      return false;
    }
    known = true;
    return true;
  }
  bool PostprocessorCore::set_scale_visible(bool visible, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(state.active_view_id);
    if (source_operation_active || view == state.views.end() || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "The scale view is unavailable.";
      return false;
    }
    if (view->second.scale_visible != visible)
    {
      view->second.scale_visible = visible;
      ++scene_revision;
    }
    return true;
  }
  bool PostprocessorCore::standard_view(const std::string& name, std::string& error)
  {
    error.clear();
    const auto view = state.views.find(state.active_view_id);
    if (view == state.views.end())
    {
      error = "The camera view is unavailable.";
      return false;
    }
    CameraDefinition base;
    if (view->second.camera)
    {
      base = *view->second.camera;
    }
    else
    {
      base.position = DisplayDefaults::initial_position;
      base.view_up = DisplayDefaults::initial_view_up;
    }
    if (name == "perspective")
    {
      if (base.parallel_projection)
      {
        // Preserve focal-plane height when returning to perspective projection.
        const double distance = std::hypot(base.position[0]-base.focal_point[0],
          base.position[1]-base.focal_point[1],base.position[2]-base.focal_point[2]);
        const double radians = std::acos(-1.0)/180.0;
        const double target = base.parallel_scale/std::tan(base.vertical_view_angle_degrees*radians/2);
        if (!std::isfinite(target) || distance <= 0 || target <= 0)
        {
          error = "Cannot preserve an invalid camera span.";
          return false;
        }
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
          base.position[axis] = base.focal_point[axis]+(base.position[axis]-base.focal_point[axis])*target/distance;
        }
      }
      base.parallel_projection = false;
      return set_camera(view->first,base,error);
    }
    CameraOperations::StandardView requested = CameraOperations::StandardView::Front;
    if (name == "front")
    {
      requested = CameraOperations::StandardView::Front;
    }
    else if (name == "back")
    {
      requested = CameraOperations::StandardView::Back;
    }
    else if (name == "top")
    {
      requested = CameraOperations::StandardView::Top;
    }
    else if (name == "bottom")
    {
      requested = CameraOperations::StandardView::Bottom;
    }
    else if (name == "left")
    {
      requested = CameraOperations::StandardView::Left;
    }
    else if (name == "right")
    {
      requested = CameraOperations::StandardView::Right;
    }
    else if (name == "iso")
    {
      requested = CameraOperations::StandardView::Isometric;
    }
    else if (name == "parallel")
    {
      requested = CameraOperations::StandardView::Orthographic;
    }
    else
    {
      error = "Unknown standard view.";
      return false;
    }
    CameraDefinition result;
    return CameraOperations::standard_view(base,requested,result,error) && set_camera(view->first,result,error);
  }
}
