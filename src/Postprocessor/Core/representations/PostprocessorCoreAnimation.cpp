#include "AnimationRotationRuntime.hpp"
#include "../application/PostprocessorCore.hpp"
#include <exception>
#include <limits>
#include <cmath>
#include <algorithm>
#include <utility>
namespace Postprocessor
{
  bool PostprocessorCore::set_animation(const std::string& source_id,const std::string& view_id,
    const AnimationDefinition& animation,std::string& error)
  {
    error.clear();
    const auto tableView = state.views.find(view_id);
    if (animation.playing && tableView != state.views.end() && tableView->second.table)
    {
      error = "Return to the 3D view before starting mesh animation.";
      return false;
    }
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        !valid_animation_rotation(animation.rotation,error) ||
        animation.frames_per_second < AnimationDefaults::minimum_fps || animation.frames_per_second > AnimationDefaults::maximum_fps)
    {
      if (error.empty())
      {
        error = "Animation requires an available source and FPS between 1 and 120.";
      }
      return false;
    }
    try
    {
      auto staged = state;
      for (auto& entry : staged.representations)
      {
        auto& display = entry.second;
        if (!display_matches_source(display,source_id,view_id))
        {
          continue;
        }
        const auto& old = display.animation;
        if (old.frames_per_second == animation.frames_per_second && old.loop == animation.loop && old.playing == animation.playing &&
            same_animation_rotation(old.rotation,animation.rotation))
        {
          return true;
        }
        if (animation.playing && !old.playing)
        {
          const auto data = result_data(source_id);
          std::optional<std::size_t> first;
          std::size_t last = 0;
          std::size_t count = 0;
          if (data)
          {
            for (std::size_t i = 0; i < data->increments.size(); ++i)
            {
              if (frame_has_mesh(source_id,i))
              {
                if (!first)
                {
                  first = i;
                }
                last = i;
                ++count;
              }
            }
          }
          if (count < 2)
          {
            error = "Animation requires at least two displayable frames.";
            return false;
          }
          if (display.increment_index >= last)
          {
            display.increment_index = *first;
          }
          staged.active_source_id = source_id;
          staged.active_representation_id = entry.first;
          for (auto& other : staged.representations)
          {
            if (other.first != entry.first && other.second.view_id == view_id)
            {
              other.second.animation.playing = false;
            }
          }
        }
        const auto id = entry.first;
        const bool reanchor = !animation.playing || !old.playing || !same_animation_rotation(old.rotation,animation.rotation);
        display.animation = animation;
        state.swap(staged);
        if (reanchor)
        {
          animationRuntime->erase(id);
        }
        ++scene_revision;
        return true;
      }
      error = "The source has no representation in the target view.";
      return false;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot change animation: ")+exception.what();
      return false;
    }
    catch (...)
    {
      error = "Cannot change animation: unknown failure.";
      return false;
    }
  }
  bool PostprocessorCore::advance_animation_rotation(const std::string& source_id,const std::string& view_id,
    double seconds,std::string& error)
  {
    error.clear();
    if (source_operation_active || !std::isfinite(seconds) || seconds < 0)
    {
      error = "Animation rotation requires a finite nonnegative elapsed time outside source operations.";
      return false;
    }
    const auto view = state.views.find(view_id);
    const auto source = result_data(source_id);
    if (view == state.views.end() || !source)
    {
      error = "The animation rotation source or view is unavailable.";
      return false;
    }
    for (const auto& entry : state.representations)
    {
      const auto& display = entry.second;
      if (!display.visible || !display_matches_source(display,source_id,view_id))
      {
        continue;
      }
      const auto& settings = display.animation;
      const bool enabled = std::any_of(settings.rotation.begin(),settings.rotation.end(),[](const auto& axis)
      {
        return axis.enabled;
      });
      if (!settings.playing || !enabled || seconds == 0)
      {
        return true;
      }
      if (!view->second.camera || view->second.fit_pending)
      {
        return true;
      }
      auto next = animationRuntime->stage(entry.first);
      if (next.source != source || next.camera_revision != view->second.camera_revision ||
          !same_animation_rotation(next.settings,settings.rotation))
      {
        next = {};
        next.base = *view->second.camera;
        next.settings = settings.rotation;
        next.source = source;
        next.camera_revision = view->second.camera_revision;
        for (std::size_t frame = 0; frame < source->increments.size(); ++frame)
        {
          if (frame_has_mesh(source_id,frame))
          {
            ++next.frames;
          }
        }
      }
      if (next.frames < 2)
      {
        error = "Animation rotation requires at least two displayable frames.";
        return false;
      }
      CameraDefinition camera;
      if (!next.advance(seconds,static_cast<double>(next.frames)/settings.frames_per_second,camera,error) ||
          !set_camera(view_id,camera,error))
      {
        return false;
      }
      next.camera_revision = state.views.at(view_id).camera_revision;
      animationRuntime->publish(entry.first,std::move(next));
      return true;
    }
    error = "The animation rotation representation is unavailable.";
    return false;
  }

}
