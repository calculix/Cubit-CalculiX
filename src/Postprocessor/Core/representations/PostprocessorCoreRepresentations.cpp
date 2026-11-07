#include "../application/PostprocessorCore.hpp"
#include <exception>
#include <limits>

namespace Postprocessor
{
  bool PostprocessorCore::source_has_mesh(const std::string& source_id) const noexcept
  {
    const auto found = runtime.find(source_id);
    return found != runtime.end() && found->second.first_mesh_increment.has_value();
  }

  std::uint64_t PostprocessorCore::display_revision() const noexcept
  {
    return scene_revision;
  }

  bool PostprocessorCore::ensure_source_representation(PostprocessorSession& staged,
    const std::string& source_id, const std::string& view_id, std::size_t increment,
    bool visible, std::string& error) const
  {
    if (staged.sources.find(source_id) == staged.sources.end() || staged.views.find(view_id) == staged.views.end())
    {
      error = "A display requires a registered source and view.";
      return false;
    }
    for (auto& entry : staged.representations)
    {
      auto& display = entry.second;
      if (display.input.owner == OutputOwner::Source && display.input.object_id == source_id &&
          display.input.port == 0 && display.view_id == view_id)
      {
        display.visible = visible;
        return true;
      }
    }
    Representation display;
    display.id = "display:" + view_id + ":" + source_id;
    display.name = staged.sources.at(source_id).name;
    display.input = OutputReference{OutputOwner::Source, source_id, 0};
    display.view_id = view_id;
    display.visible = visible;
    display.increment_index = increment;
    if (!staged.representations.emplace(display.id, display).second)
    {
      error = "The source display identifier is already in use.";
      return false;
    }
    return true;
  }

  bool PostprocessorCore::prepare_initial_display(PostprocessorSession& staged,
    const std::string& source_id, const std::optional<std::size_t>& increment, std::string& error) const
  {
    if (!increment)
    {
      return true;
    }
    if (!ensure_source_representation(staged, source_id, staged.active_view_id, *increment, true, error))
    {
      return false;
    }
    for (const auto& entry : staged.representations)
    {
      if (entry.second.input.owner == OutputOwner::Source && entry.second.input.object_id == source_id &&
          entry.second.view_id == staged.active_view_id && entry.second.input.port == 0)
      {
        staged.active_representation_id = entry.first;
        break;
      }
    }
    return true;
  }

  bool PostprocessorCore::set_source_visible(const std::string& source_id, const std::string& view_id,
    bool visible, std::string& error)
  {
    error.clear();
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Cannot change displays during a source operation or after revision exhaustion.";
      return false;
    }
    if (state.sources.find(source_id) == state.sources.end() || state.views.find(view_id) == state.views.end())
    {
      error = "The source or target view is not registered.";
      return false;
    }
    const auto loaded = runtime.find(source_id);
    if (visible && (loaded == runtime.end() || !loaded->second.first_mesh_increment))
    {
      error = "The source has no displayable mesh increment.";
      return false;
    }
    try
    {
      auto staged = state;
      bool found = false;
      bool changed = false;
      for (auto& entry : staged.representations)
      {
        auto& display = entry.second;
        if (display.input.owner == OutputOwner::Source && display.input.object_id == source_id &&
            display.view_id == view_id && display.input.port == 0)
        {
          found = true;
          changed = display.visible != visible;
          display.visible = visible;
          break;
        }
      }
      if (!found && visible)
      {
        if (!ensure_source_representation(staged, source_id, view_id, *loaded->second.first_mesh_increment, true, error))
        {
          return false;
        }
        changed = true;
      }
      if (changed)
      {
        state.swap(staged);
        ++scene_revision;
      }
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot change source visibility: ") + exception.what();
      return false;
    }
  }
}
