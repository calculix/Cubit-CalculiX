#include "../application/PostprocessorCore.hpp"
#include <exception>
#include <limits>
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
        animation.frames_per_second < AnimationDefaults::minimum_fps || animation.frames_per_second > AnimationDefaults::maximum_fps)
    {
      error = "Animation requires an available source and FPS between 1 and 120.";
      return false;
    }
    try
    {
      auto staged = state;
      for (auto& entry : staged.representations)
      {
        auto& display = entry.second;
        if (display.input.owner != OutputOwner::Source || display.input.object_id != source_id ||
            display.input.port != 0 || display.view_id != view_id)
        {
          continue;
        }
        const auto& old = display.animation;
        if (old.frames_per_second == animation.frames_per_second && old.loop == animation.loop && old.playing == animation.playing)
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
        display.animation = animation;
        state.swap(staged);
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
}
