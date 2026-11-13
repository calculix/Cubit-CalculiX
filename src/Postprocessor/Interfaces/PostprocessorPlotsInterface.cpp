#include "PostprocessorApplicationInterface.hpp"
#include <algorithm>
bool PostprocessorApplicationInterface::create_plot(const std::string& name,std::string& error)
{
  return core.create_plot(name,error);
}
bool PostprocessorApplicationInterface::add_plot_entity(const std::string& plotName,const std::string& name,
  const std::string& sourceName,const std::optional<DatasetSelector>& selector,Postprocessor::EntityKind kind,
  std::int64_t id,std::optional<std::int64_t> ip,const std::string& fieldName,const std::string& component,
  bool magnitude,Postprocessor::PlotReduction reduction,std::string& error)
{
  using namespace Postprocessor;
  error.clear();
  const auto plot = core.plot_id_from_name(plotName);
  if (plot.empty())
  {
    error = "Create/select a plot first.";
    return false;
  }
  const auto& state = core.session();
  const auto source = sourceName.empty() ? state.active_source_id : core.source_id_from_name(sourceName);
  const auto data = core.result_data(source);
  if (!data || id < 0 || (kind == EntityKind::IntegrationPoint && (!ip || *ip < 1 || *ip > 2147483647)))
  {
    error = "The original entity/source is unavailable.";
    return false;
  }
  std::optional<std::size_t> dataset;
  if (selector)
  {
    dataset = resolve_dataset(source,*selector,error);
    if (!dataset)
    {
      return false;
    }
  }
  std::optional<FieldSelection> field;
  if (fieldName.empty())
  {
    const auto settings = coloring(sourceName,error);
    if (settings)
    {
      field = settings->field;
    }
  }
  else
  {
    for (const auto& group : core.field_groups(source,std::nullopt))
    {
      if ((group.key == fieldName || group.field.name == fieldName || group.field.array_name == fieldName) &&
          (!dataset || std::find(group.datasets.begin(),group.datasets.end(),*dataset) != group.datasets.end()))
      {
        if (field && field->group_key != group.key)
        {
          error = "Ambiguous field; specify its exact key and dataset.";
          return false;
        }
        const bool useMagnitude = component.empty() ? group.field.vector_components.has_value() : magnitude;
        field = FieldSelection{dataset,group.field.array_name,group.field.association,
          useMagnitude ? std::string{} : component.empty() && !group.field.components.empty() ? group.field.components.front() : component,
          useMagnitude,group.key};
      }
    }
  }
  if (!field)
  {
    error = "Select a spatial field/component for the history.";
    return false;
  }
  if (!component.empty())
  {
    field->magnitude = magnitude;
    field->component = magnitude ? std::string{} : component;
  }
  std::optional<PlotEntityReference> reference;
  for (const auto& part : core.mesh_datasets(source))
  {
    if (dataset && *dataset != part.id)
    {
      continue;
    }
    PlotEntityReference candidate;
    candidate.entity = {source,part.id,kind,id,ip};
    candidate.dataset_kind = part.kind;
    candidate.dataset_name = part.name;
    candidate.model_id = part.model_id;
    candidate.field = *field;
    candidate.field.dataset_id = part.id;
    candidate.reduction = reduction;
    PlotCurveData values;
    std::string issue;
    if (!plot_entity_data(*data,candidate,values,issue))
    {
      error = issue;
      return false;
    }
    if (std::any_of(values.samples.begin(),values.samples.end(),[](const auto& sample)
      {
        return sample.valid;
      }))
    {
      if (reference)
      {
        error = "The entity occurs in multiple datasets; specify a dataset.";
        return false;
      }
      reference = candidate;
    }
  }
  if (!reference)
  {
    error = "No valid original samples match this entity/field.";
    return false;
  }
  PlotCurveDefinition curve;
  curve.name = name;
  curve.input = *reference;
  return core.add_plot_curve(plot,curve,error);
}
bool PostprocessorApplicationInterface::add_plot_table(const std::string& plotName,const std::string& name,
  const std::string& sourceName,const std::string& tableName,const std::string& x,const std::string& y,
  const std::string& column,const std::string& value,std::string& error)
{
  const auto source = sourceName.empty() ? core.session().active_source_id : core.source_id_from_name(sourceName);
  std::optional<std::string> key;
  for (const auto& table : tables(source))
  {
    if (table.key == tableName || table.name == tableName)
    {
      if (key)
      {
        error = "The table name is ambiguous; use its exact key.";
        return false;
      }
      key = table.key;
    }
  }
  if (!key)
  {
    error = "No table has that name/key.";
    return false;
  }
  Postprocessor::PlotCurveDefinition curve;
  curve.name = name;
  curve.input = Postprocessor::PlotTableReference{source,*key,x,y,column,value};
  return core.add_plot_curve(core.plot_id_from_name(plotName),curve,error);
}
bool PostprocessorApplicationInterface::change_plot(const std::string& name,const std::string& action,
  const std::string& value,std::string& error)
{
  return core.change_plot(core.plot_id_from_name(name),action,value,error);
}
bool PostprocessorApplicationInterface::change_plot_curve(const std::string& plot,const std::string& name,
  const std::string& action,std::string& error)
{
  return core.change_plot_curve(core.plot_id_from_name(plot),name,action,error);
}
bool PostprocessorApplicationInterface::set_plot_axes(const std::string& name,const Postprocessor::PlotAxes& axes,std::string& error)
{
  return core.set_plot_axes(core.plot_id_from_name(name),axes,error);
}
bool PostprocessorApplicationInterface::plot_data(const std::string& id,Postprocessor::PlotData& output,std::string& error) const
{
  return core.plot_data(id,output,error);
}
bool PostprocessorApplicationInterface::export_plot(const std::string& name,const std::string& path,std::string& error) const
{
  const auto id = core.plot_id_from_name(name);
  const auto found = core.session().plots.find(id);
  Postprocessor::PlotData data;
  if (found == core.session().plots.end())
  {
    error = "The plot is unavailable.";
    return false;
  }
  return core.plot_data(id,data,error) && Postprocessor::export_plot_csv(found->second,data,path,error);
}
