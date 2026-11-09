#include "../application/PostprocessorCore.hpp"
#include <algorithm>
#include <exception>
#include <limits>

namespace Postprocessor
{
  std::vector<SimResults::Dataset> PostprocessorCore::mesh_datasets(const std::string& source_id) const
  {
    const auto loaded = runtime.find(source_id);
    return loaded == runtime.end() ? std::vector<SimResults::Dataset>{} : loaded->second.datasets;
  }

  bool PostprocessorCore::set_dataset_visibility(const std::string& source_id, const std::string& view_id,
    DatasetVisibilityAction action, std::optional<std::size_t> dataset, std::string& error)
  {
    error.clear();
    const auto loaded = runtime.find(source_id);
    if (source_operation_active || scene_revision == std::numeric_limits<std::uint64_t>::max() ||
        loaded == runtime.end() || !loaded->second.first_mesh_increment ||
        state.sources.count(source_id) == 0 || state.views.count(view_id) == 0)
    {
      error = "Dataset visibility requires a loaded mesh source/view and no active source operation.";
      return false;
    }
    if ((action != DatasetVisibilityAction::Show && action != DatasetVisibilityAction::Hide &&
        action != DatasetVisibilityAction::ShowOnly && action != DatasetVisibilityAction::ShowAll) ||
        ((action == DatasetVisibilityAction::ShowAll) == dataset.has_value()))
    {
      error = "Dataset visibility requires a valid action and dataset selector (except Show all).";
      return false;
    }
    const auto& catalogue = loaded->second.datasets;
    if (dataset && std::none_of(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value)
        {
          return value.id == *dataset;
        }))
    {
      error = "The dataset ID is not a mesh dataset in this source.";
      return false;
    }
    try
    {
      auto staged = state;
      auto found = std::find_if(staged.representations.begin(),staged.representations.end(),[&](const auto& entry)
      {
        return entry.second.input.owner == OutputOwner::Source && entry.second.input.object_id == source_id &&
          entry.second.input.port == 0 && entry.second.view_id == view_id;
      });
      const bool created = found == staged.representations.end();
      if (created)
      {
        if (!ensure_source_representation(staged,source_id,view_id,*loaded->second.first_mesh_increment,false,error))
        {
          return false;
        }
        found = std::find_if(staged.representations.begin(),staged.representations.end(),[&](const auto& entry)
        {
          return entry.second.input.owner == OutputOwner::Source && entry.second.input.object_id == source_id &&
            entry.second.input.port == 0 && entry.second.view_id == view_id;
        });
      }
      auto& display = found->second;
      const auto oldHidden = display.hidden_datasets;
      const bool oldVisible = display.visible;
      if (action == DatasetVisibilityAction::Hide)
      {
        display.hidden_datasets.insert(*dataset);
      }
      else
      {
        display.visible = true;
        if (action == DatasetVisibilityAction::Show)
        {
          display.hidden_datasets.erase(*dataset);
        }
        else
        {
          display.hidden_datasets.clear();
          if (action == DatasetVisibilityAction::ShowOnly)
          {
            for (const auto& value : catalogue)
            {
              if (value.id != *dataset)
              {
                display.hidden_datasets.insert(value.id);
              }
            }
          }
        }
      }
      if (created || oldHidden != display.hidden_datasets || oldVisible != display.visible)
      {
        state.swap(staged);
        ++scene_revision;
      }
      return true;
    }
    catch (const std::exception& exception)
    {
      error = std::string("Cannot change dataset visibility: ")+exception.what();
    }
    catch (...)
    {
      error = "Cannot change dataset visibility: unknown failure.";
    }
    return false;
  }
}
