#include "../application/PostprocessorCore.hpp"
#include "ResultMesh.hpp"
#include <exception>
#include <algorithm>
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
    display.coloring.palette = default_palette;
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
    staged.views.at(staged.active_view_id).table.reset();
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

  bool PostprocessorCore::frame_has_mesh(const std::string& source_id, std::size_t increment_index) const
  {
    const auto loaded = runtime.find(source_id);
    return loaded != runtime.end() && loaded->second.data &&
      increment_index < loaded->second.data->increments.size() &&
      !result_meshes(loaded->second.data->increments[increment_index].dataset.get()).empty();
  }

  bool PostprocessorCore::set_frame(const std::string& source_id, const std::string& view_id,
    std::size_t increment_index, std::string& error)
  {
    error.clear();
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        state.sources.find(source_id) == state.sources.end() || state.views.find(view_id) == state.views.end())
    {
      error = "Cannot select a frame for this source/view during the current operation.";
      return false;
    }
    try
    {
      const auto data = result_data(source_id);
      const auto& table = state.views.at(view_id).table;
      if (table && !state.views.at(view_id).plot_active && table->source_id == source_id)
      {
        if (!data || increment_index >= data->increments.size())
        {
          error = "The requested table frame is unavailable.";
          return false;
        }
        auto requested = *table;
        requested.frame = increment_index;
        if (result_table_data(data,requested).rows > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        {
          error = "This table frame exceeds the current Qt table view row limit.";
          return false;
        }
        if (table->frame != increment_index)
        {
          auto staged = state;
          staged.views.at(view_id).table->frame = increment_index;
          state.swap(staged);
          ++scene_revision;
        }
        return true;
      }
      if (!frame_has_mesh(source_id, increment_index))
      {
        error = "The requested frame is unavailable or has no displayable mesh.";
        return false;
      }
      for (auto& entry : state.representations)
      {
        auto& display = entry.second;
        if (display.input.owner == OutputOwner::Source && display.input.object_id == source_id &&
            display.input.port == 0 && display.view_id == view_id)
        {
          if (display.increment_index != increment_index)
          {
            display.increment_index = increment_index;
            ++scene_revision;
          }
          return true;
        }
      }
      // Prepare a hidden representation if the source is not displayed here.
      auto staged = state;
      if (!ensure_source_representation(staged, source_id, view_id, increment_index, false, error))
      {
        return false;
      }
      state.swap(staged);
      ++scene_revision;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot select Postprocessor frame: ") + exception.what();
      return false;
    }
    catch (...)
    {
      error = "Cannot select Postprocessor frame: unknown failure.";
      return false;
    }
  }

  bool PostprocessorCore::set_coloring(const std::string& source_id, const std::string& view_id,
    const ResultColoring& coloring, std::string& error, bool activateResult)
  {
    error.clear();
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        !valid_coloring(coloring, error))
    {
      if (error.empty())
      {
        error = "Cannot change coloring during the current operation.";
      }
      return false;
    }
    for (auto& entry : state.representations)
    {
      auto& display = entry.second;
      if (display.input.owner == OutputOwner::Source && display.input.object_id == source_id &&
          display.input.port == 0 && display.view_id == view_id)
      {
        try
        {
          const bool closesTable = activateResult && state.views.at(view_id).table.has_value();
          const bool changesActive = activateResult && (state.active_source_id != source_id ||
            state.active_view_id != view_id || state.active_representation_id != entry.first);
          if (same_coloring(display.coloring,coloring) && !closesTable && !changesActive)
          {
            return true;
          }
          if (coloring.field && (activateResult || !same_field(display.coloring.field,coloring.field)))
          {
            const auto data = result_data(source_id);
            const auto* field = data && display.increment_index < data->increments.size() ?
              find_field(data->increments[display.increment_index],*coloring.field) : nullptr;
            if (!field || field->association == SimResults::Association::Table ||
                (coloring.field->magnitude && !field->vector_components) ||
                (!coloring.field->magnitude && std::find(field->components.begin(),field->components.end(),coloring.field->component) == field->components.end()))
            {
              error = "The requested field/component is unavailable in this frame.";
              return false;
            }
          }
          auto staged = state;
          staged.representations.at(entry.first).coloring = coloring;
          if (activateResult)
          {
            staged.views.at(view_id).table.reset();
            staged.active_view_id = view_id;
            staged.active_source_id = source_id;
            staged.active_representation_id = entry.first;
          }
          state.swap(staged);
          ++scene_revision;
          return true;
        }
        catch (const std::exception& exception)
        {
          error = std::string("Cannot change result coloring: ") + exception.what();
          return false;
        }
      }
    }
    error = "The source has no representation in the target view.";
    return false;
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
