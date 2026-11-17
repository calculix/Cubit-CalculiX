#include "../application/PostprocessorCore.hpp"
#include <algorithm>
#include <limits>
#include <exception>
namespace Postprocessor
{
  DatasetAppearance PostprocessorCore::dataset_appearance(const std::string& source,const std::string& view,std::size_t dataset) const
  {
    const auto catalogue = mesh_datasets(source);
    const auto item = std::find_if(catalogue.begin(),catalogue.end(),[&](const auto& value)
    {
      return value.id == dataset;
    });
    SimResults::Dataset unknown;
    DatasetAppearance value = default_appearance(unknown);
    if (item != catalogue.end())
    {
      value = default_appearance(*item);
    }
    for (const auto& entry : state.representations)
    {
      const auto& display = entry.second;
      if (display.input.owner == OutputOwner::Source && display.input.object_id == source && display.view_id == view)
      {
        const auto found = display.dataset_appearance.find(dataset);
        if (found != display.dataset_appearance.end())
        {
          return found->second;
        }
      }
    }
    return value;
  }
  bool PostprocessorCore::change_dataset_appearance(const std::vector<AppearanceTarget>& targets,
    const AppearancePatch& patch,std::string& error,bool validate_only)
  {
    error.clear();
    if (source_operation_active || targets.empty() || scene_revision == std::numeric_limits<std::uint64_t>::max())
    {
      error = "Appearance requires selected datasets and an idle source operation.";
      return false;
    }
    try
    {
      auto staged = state;
      auto stagedBindings = saved_bindings;
      bool changed = false;
      for (const auto& target : targets)
      {
        const auto catalogue = mesh_datasets(target.source_id);
        const auto dataset = std::find_if(catalogue.begin(),catalogue.end(),[&](const auto& value)
        {
          return value.id == target.dataset_id;
        });
        const auto owner = std::find_if(staged.representations.begin(),staged.representations.end(),[&](const auto& entry)
        {
          return entry.second.input.owner == OutputOwner::Source && entry.second.input.object_id == target.source_id &&
            entry.second.view_id == target.view_id;
        });
        if (dataset == catalogue.end() || owner == staged.representations.end())
        {
          error = "A selected dataset or source representation is unavailable.";
          return false;
        }
        auto& durable = stagedBindings[owner->first].appearance;
        const auto oldCount = durable.size();
        durable.erase(std::remove_if(durable.begin(),durable.end(),[&](const auto& saved)
        {
          bool identity = saved.dataset.name == dataset->name;
          if (saved.dataset.model_id >= 0)
          {
            identity = saved.dataset.model_id == dataset->model_id;
          }
          return saved.dataset.kind == dataset->kind && identity;
        }),durable.end());
        changed = changed || durable.size() != oldCount;
        auto& overrides = owner->second.dataset_appearance;
        if (patch.reset)
        {
          changed = overrides.erase(target.dataset_id) != 0 || changed;
          continue;
        }
        auto value = dataset_appearance(target.source_id,target.view_id,target.dataset_id);
        apply_appearance_patch(patch,value);
        if (!valid_appearance(*dataset,value,error))
        {
          return false;
        }
        const auto prior = overrides.find(target.dataset_id);
        if (prior == overrides.end() || !same_appearance(prior->second,value))
        {
          overrides[target.dataset_id] = value;
          changed = true;
        }
      }
      if (!validate_only && changed)
      {
        state.swap(staged);
        saved_bindings.swap(stagedBindings);
        ++scene_revision;
      }
      return true;
    }
    catch (const std::exception& failure)
    {
      error = std::string("Cannot change dataset appearance: ")+failure.what();
      return false;
    }
  }
}
