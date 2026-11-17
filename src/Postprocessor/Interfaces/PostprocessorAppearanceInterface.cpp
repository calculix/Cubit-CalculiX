#include "PostprocessorApplicationInterface.hpp"
#include <utility>
Postprocessor::DatasetAppearance PostprocessorApplicationInterface::dataset_appearance(
  const std::string& source,const std::string& view,std::size_t dataset) const
{
  return core.dataset_appearance(source,view,dataset);
}
bool PostprocessorApplicationInterface::change_dataset_appearance(const std::vector<Postprocessor::AppearanceTarget>& targets,
  const Postprocessor::AppearancePatch& patch,std::string& error,bool validate_only)
{
  return core.change_dataset_appearance(targets,patch,error,validate_only);
}
bool PostprocessorApplicationInterface::resolve_appearance_targets(const std::string& source,
  Postprocessor::DatasetCategory category,const std::vector<std::string>& names,
  std::vector<Postprocessor::AppearanceTarget>& targets,std::string& error) const
{
  std::string id = core.session().active_source_id;
  if (!source.empty())
  {
    id = core.source_id_from_name(source);
  }
  const auto catalogue = core.mesh_datasets(id);
  std::vector<Postprocessor::AppearanceTarget> candidate;
  for (const auto& name : names)
  {
    std::optional<std::size_t> found;
    for (const auto& dataset : catalogue)
    {
      if (dataset.name == name && Postprocessor::dataset_in_category(dataset,category))
      {
        if (found)
        {
          error = "Appearance dataset name is ambiguous within its category: "+name;
          return false;
        }
        found = dataset.id;
      }
    }
    if (!found)
    {
      error = "Appearance dataset not found in the selected source/category: "+name;
      return false;
    }
    candidate.push_back({id,core.session().active_view_id,*found});
  }
  if (candidate.empty())
  {
    error = "Appearance requires at least one dataset name.";
    return false;
  }
  targets = std::move(candidate);
  return true;
}
