#include "../application/PostprocessorCore.hpp"
#include <algorithm>
#include <exception>
#include <limits>
namespace Postprocessor
{
  bool PostprocessorCore::set_category_visible(const std::string& source, const std::string& view,
    DatasetCategory category, bool visible, std::string& error)
  {
    error.clear();
    const auto loaded = runtime.find(source);
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        loaded == runtime.end() || !loaded->second.first_mesh_increment || state.views.count(view) == 0)
    {
      error = "Category visibility requires an available mesh source/view.";
      return false;
    }
    try
    {
      std::vector<std::size_t> ids;
      for (const auto& data : loaded->second.datasets)
      {
        if (dataset_in_category(data,category))
        {
          ids.push_back(data.id);
        }
      }
      if (ids.empty())
      {
        error = "The source has no datasets in this category.";
        return false;
      }
      auto staged = state;
      auto find = [&]()
      {
        return std::find_if(staged.representations.begin(),staged.representations.end(),[&](const auto& item)
        {
          return item.second.view_id == view && item.second.input.owner == OutputOwner::Source &&
            item.second.input.object_id == source && item.second.input.port == 0;
        });
      };
      bool created = find() == staged.representations.end();
      if (created && !ensure_source_representation(staged,source,view,*loaded->second.first_mesh_increment,false,error))
      {
        return false;
      }
      auto& display = find()->second;
      const auto old = display.hidden_datasets;
      const bool oldVisible = display.visible;
      for (auto id : ids)
      {
        if (visible)
        {
          display.hidden_datasets.erase(id);
        }
        else
        {
          display.hidden_datasets.insert(id);
        }
      }
      display.visible = visible || display.visible;
      if (created || old != display.hidden_datasets || oldVisible != display.visible)
      {
        state.swap(staged);
        ++scene_revision;
      }
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot change category visibility: ")+exception.what();
      return false;
    }
  }
}

namespace Postprocessor
{
  bool PostprocessorCore::show_only_source(const std::string& source, const std::string& view, std::string& error)
  {
    error.clear();
    const auto loaded = runtime.find(source);
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        loaded == runtime.end() || !loaded->second.first_mesh_increment || state.views.count(view) == 0)
    {
      error = "Source isolation requires an available mesh source and view.";
      return false;
    }
    try
    {
      auto staged = state;
      staged.views.at(view).table.reset();
      if (!ensure_source_representation(staged,source,view,*loaded->second.first_mesh_increment,true,error))
      {
        return false;
      }
      for (auto& item : staged.representations)
      {
        auto& display = item.second;
        if (display.view_id == view && display.input.owner == OutputOwner::Source)
        {
          display.visible = display.input.object_id == source;
          display.animation.playing = false;
        }
      }
      state.swap(staged);
      ++scene_revision;
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot isolate source: ")+exception.what();
      return false;
    }
  }
}
