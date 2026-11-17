#include <type_traits>
#include "PostprocessorApplicationInterface.hpp"
#include <algorithm>
bool PostprocessorApplicationInterface::change_threshold(const std::string& name,
  const Postprocessor::ThresholdDefinition& requested,std::string& error)
{
  return core.change_threshold(name,requested,error);
}
bool PostprocessorApplicationInterface::threshold_field(const std::string& sourceName,
  const Postprocessor::ResultFieldSelector& selector,const std::string& component,bool magnitude,
  std::optional<std::size_t> dataset,Postprocessor::ThresholdDefinition& requested,std::string& error) const
{
  std::string source = core.session().active_source_id;
  if (!sourceName.empty())
  {
    source = core.source_id_from_name(sourceName);
  }
  if (dataset && requested.field.dataset_id == dataset && !requested.dataset_kind.empty())
  {
    std::optional<std::size_t> rebound;
    for (const auto& metadata : core.mesh_datasets(source))
    {
      if (metadata.kind == requested.dataset_kind && metadata.name == requested.dataset_name &&
          metadata.model_id == requested.dataset_model)
      {
        if (rebound)
        {
          error = "The threshold dataset identity is ambiguous.";
          return false;
        }
        rebound = metadata.id;
      }
    }
    if (!rebound)
    {
      error = "The saved threshold dataset is unavailable.";
      return false;
    }
    dataset = rebound;
  }
  Postprocessor::ResultFieldGroup match;
  if (!Postprocessor::resolve_result_field(core.field_groups(source,std::nullopt),selector,dataset,
      component,magnitude,false,match,error))
  {
    return false;
  }
  if (!match.unit_issue.empty())
  {
    error = match.unit_issue;
    return false;
  }
  auto candidate = requested;
  candidate.field = {dataset,match.field.array_name,match.field.association,component,magnitude,match.key};
  candidate.dataset_kind.clear();
  candidate.dataset_name.clear();
  candidate.dataset_model = -1;
  if (dataset)
  {
    const auto metadata = core.mesh_datasets(source);
    const auto found = std::find_if(metadata.begin(),metadata.end(),[&](const auto& entry)
    {
      return entry.id == *dataset;
    });
    if (found == metadata.end())
    {
      error = "The threshold dataset is unavailable.";
      return false;
    }
    candidate.dataset_kind = found->kind;
    candidate.dataset_name = found->name;
    candidate.dataset_model = found->model_id;
  }
  if (!Postprocessor::bind_threshold_units(candidate,match.field,error))
  {
    return false;
  }
  requested = std::move(candidate);
  return true;
}
std::string PostprocessorApplicationInterface::output_source(const Postprocessor::OutputReference& input) const
{
  return core.output_source(input);
}
bool PostprocessorApplicationInterface::display_matches_source(const Postprocessor::Representation& display,
  const std::string& source,const std::string& view) const
{
  return core.display_matches_source(display,source,view);
}
bool PostprocessorApplicationInterface::filter_motion_range(const std::string& name,std::array<double,2>& range,std::string& error) const
{
  return core.filter_motion_range(name,core.session().active_view_id,range,error);
}
bool PostprocessorApplicationInterface::create_filter(Postprocessor::FilterDefinition requested,const std::string& sourceName,std::string& error)
{
  requested.input = {Postprocessor::OutputOwner::Source,[&]() -> std::common_type_t<decltype(core.session().active_source_id), decltype(core.source_id_from_name(sourceName))> {
    if (sourceName.empty()) {
      return core.session().active_source_id;
    } else {
      return core.source_id_from_name(sourceName);
    }
  }(),0};
  return core.create_filter(requested,core.session().active_view_id,error);
}
bool PostprocessorApplicationInterface::change_filter(const std::string& name,const std::string& action,const std::string& view,
  const Postprocessor::PlaneDefinition& plane,const std::string& value,std::string& error)
{
  return core.change_filter(name,action,[&]() -> std::common_type_t<decltype(core.session().active_view_id), decltype(view)> {
    if (view.empty()) {
      return core.session().active_view_id;
    } else {
      return view;
    }
  }(),plane,value,error);
}
bool PostprocessorApplicationInterface::evaluate_output(const Postprocessor::Representation& display,
  std::shared_ptr<const Postprocessor::EvaluatedOutput>& output,std::string& error) const
{
  return core.evaluate_output(display,output,error);
}

std::string PostprocessorApplicationInterface::suggested_filter_name(Postprocessor::FilterKind kind) const
{
  const std::string prefix = [&]() -> std::string {
    if (kind == Postprocessor::FilterKind::Threshold)
    {
      return Postprocessor::FilterDefaults::threshold_name_prefix;
    }
    if (kind == Postprocessor::FilterKind::Clip) {
      return Postprocessor::FilterDefaults::clip_name_prefix;
    } else {
      return Postprocessor::FilterDefaults::section_name_prefix;
    }
  }();
  std::size_t number = 1;
  while (std::any_of(core.session().filters.begin(),core.session().filters.end(),[&](const auto& entry)
    {
      return entry.second.name == prefix+std::to_string(number);
    }))
  {
    ++number;
  }
  return prefix+std::to_string(number);
}

std::shared_ptr<vtkPolyData> PostprocessorApplicationInterface::filtered_entity_edges(const Postprocessor::Representation& display,
  const Postprocessor::EntityReference& entity,std::string& error) const
{
  return core.filtered_entity_edges(display,entity,error);
}
