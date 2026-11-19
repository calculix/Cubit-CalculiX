#include <type_traits>
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "PostprocessorApplicationInterface.hpp"
#include <utility>
#include <algorithm>

bool PostprocessorApplicationInterface::add_label(const std::string& view, const std::string& name,
  const std::string& sourceName, const std::optional<DatasetSelector>& selector, Postprocessor::EntityKind kind,
  std::int64_t id, std::optional<std::int64_t> ip, const std::string& fieldName,
  const std::string& component, bool magnitude, std::string& assigned, std::string& error,
  const Postprocessor::FieldQualifiers& qualifiers, std::optional<std::int64_t> owner)
{
  error.clear();
  if (fieldName.empty() && (qualifiers.origin || qualifiers.association || qualifiers.set_name))
  {
    error = "Origin/association/set requires an explicit field name.";
    return false;
  }
  const auto& state = core->session();
  const auto viewId = [&]() -> std::common_type_t<decltype(state.active_view_id), decltype(view)> {
    if (view.empty()) {
      return state.active_view_id;
    } else {
      return view;
    }
  }();
  const auto source = [&]() -> std::common_type_t<decltype(state.active_source_id), decltype(core->source_id_from_name(sourceName))> {
    if (sourceName.empty()) {
      return state.active_source_id;
    } else {
      return core->source_id_from_name(sourceName);
    }
  }();
  std::optional<std::size_t> dataset;
  if (selector)
  {
    dataset = resolve_dataset(source,*selector,error);
    if (!dataset)
    {
      return false;
    }
  }
  const auto data = core->result_data(source);
  for (const auto& entry : state.representations)
  {
    const auto& display = entry.second;
    if (!core->display_matches_source(display,source,viewId) || !data || display.increment_index >= data->increments.size())
    {
      continue;
    }
    auto field = display.coloring.field;
    if (!fieldName.empty())
    {
      field.reset();
      Postprocessor::ResultFieldGroup group;
      if (!Postprocessor::resolve_result_field(core->field_groups(source,std::nullopt),{fieldName,qualifiers},dataset,
          component,magnitude,false,group,error))
      {
        return false;
      }
      field = Postprocessor::preferred_field_selection(group.field, group.key);
      field->dataset_id = dataset;
      if (!component.empty())
      {
        field->magnitude = magnitude;
        field->component = magnitude ? std::string{} : component;
      }
    }
    else if (field && !component.empty())
    {
      if (magnitude) {
        field->component = std::string{};
      } else {
        field->component = component;
      }
      field->magnitude = magnitude;
    }
    if (!field)
    {
      error = "Select a result field before creating a label.";
      return false;
    }
    std::optional<Postprocessor::ResultAnnotation> candidate;
    for (const auto& part : core->mesh_datasets(source))
    {
      if (dataset && part.id != *dataset)
      {
        continue;
      }
      auto copy = display;
      auto selected = *field;
      selected.dataset_id = part.id;
      copy.coloring.field = selected;
      const Postprocessor::EntityReference entity{source,part.id,kind,id,ip,owner};
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
      return core->add_label(viewId,*candidate,assigned,error);
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
  return core->change_label([&]() -> std::common_type_t<decltype(core->session().active_view_id), decltype(view)> {
    if (view.empty()) {
      return core->session().active_view_id;
    } else {
      return view;
    }
  }(),name,action,replacement,position,error);
}
bool PostprocessorApplicationInterface::set_labels(const std::string& view, bool extrema, bool visible, std::string& error)
{
  return core->set_labels([&]() -> std::common_type_t<decltype(core->session().active_view_id), decltype(view)> {
    if (view.empty()) {
      return core->session().active_view_id;
    } else {
      return view;
    }
  }(),extrema,visible,error);
}
std::vector<Postprocessor::EvaluatedAnnotation> PostprocessorApplicationInterface::annotations(const std::string& view) const
{
  return core->annotations(view);
}
