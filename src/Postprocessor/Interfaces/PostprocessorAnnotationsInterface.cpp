#include "PostprocessorApplicationInterface.hpp"
#include <utility>
#include <algorithm>

bool PostprocessorApplicationInterface::add_label(const std::string& view, const std::string& name,
  const std::string& sourceName, const std::optional<DatasetSelector>& selector, Postprocessor::EntityKind kind,
  std::int64_t id, std::optional<std::int64_t> ip, const std::string& fieldName,
  const std::string& component, bool magnitude, std::string& assigned, std::string& error)
{
  error.clear();
  const auto& state = core.session();
  const auto viewId = view.empty() ? state.active_view_id : view;
  const auto source = sourceName.empty() ? state.active_source_id : core.source_id_from_name(sourceName);
  std::optional<std::size_t> dataset;
  if (selector)
  {
    dataset = resolve_dataset(source,*selector,error);
    if (!dataset)
    {
      return false;
    }
  }
  const auto data = core.result_data(source);
  for (const auto& entry : state.representations)
  {
    const auto& display = entry.second;
    if (display.view_id != viewId || display.input.owner != Postprocessor::OutputOwner::Source ||
        display.input.object_id != source || !data || display.increment_index >= data->increments.size())
    {
      continue;
    }
    auto field = display.coloring.field;
    if (!fieldName.empty())
    {
      field.reset();
      for (const auto& group : core.field_groups(source,display.increment_index))
      {
        const auto& value = group.field;
        if ((group.key == fieldName || value.name == fieldName || value.array_name == fieldName) &&
            (!dataset || std::find(group.datasets.begin(),group.datasets.end(),*dataset) != group.datasets.end()))
        {
          if (field && field->group_key != group.key)
          {
            error = "The label field is ambiguous; use its exact field key.";
            return false;
          }
          const bool useMagnitude = component.empty() ? value.vector_components.has_value() : magnitude;
          field = Postprocessor::FieldSelection{dataset,value.array_name,value.association,
            component.empty() && !useMagnitude && !value.components.empty() ? value.components.front() : component,
            useMagnitude,group.key};
        }
      }
    }
    else if (field && !component.empty())
    {
      field->component = magnitude ? std::string{} : component;
      field->magnitude = magnitude;
    }
    if (!field)
    {
      error = "Select a result field before creating a label.";
      return false;
    }
    std::optional<Postprocessor::ResultAnnotation> candidate;
    for (const auto& part : core.mesh_datasets(source))
    {
      if (dataset && part.id != *dataset)
      {
        continue;
      }
      auto copy = display;
      auto selected = *field;
      selected.dataset_id = part.id;
      copy.coloring.field = selected;
      const Postprocessor::EntityReference entity{source,part.id,kind,id,ip};
      const auto probe = Postprocessor::annotation_probe(*data,copy,entity);
      if (probe.available && probe.values && probe.displayed_position)
      {
        if (candidate)
        {
          error = "The entity is present in multiple datasets; specify a dataset.";
          return false;
        }
        Postprocessor::ResultAnnotation label;
        label.name = name;
        label.entity = entity;
        label.field = selected;
        candidate = label;
      }
    }
    if (candidate)
    {
      return core.add_label(viewId,*candidate,assigned,error);
    }
    error = "No original entity with the requested result was found.";
    return false;
  }
  error = "The label source/view is unavailable.";
  return false;
}
bool PostprocessorApplicationInterface::change_label(const std::string& view, const std::string& name,
  const std::string& action, const std::string& replacement, const std::array<double,2>& position, std::string& error)
{
  return core.change_label(view.empty() ? core.session().active_view_id : view,name,action,replacement,position,error);
}
bool PostprocessorApplicationInterface::set_labels(const std::string& view, bool extrema, bool visible, std::string& error)
{
  return core.set_labels(view.empty() ? core.session().active_view_id : view,extrema,visible,error);
}
std::vector<Postprocessor::EvaluatedAnnotation> PostprocessorApplicationInterface::annotations(const std::string& view) const
{
  return core.annotations(view);
}
