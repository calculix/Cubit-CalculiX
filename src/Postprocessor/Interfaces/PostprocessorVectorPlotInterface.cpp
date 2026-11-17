#include "PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/representations/ResultVectors.hpp"
std::optional<Postprocessor::VectorPlotDefinition> PostprocessorApplicationInterface::vector_plot(
  const std::string& name,std::string& error) const
{
  error.clear();
  std::string id = core.session().active_source_id;
  if (!name.empty())
  {
    id = core.source_id_from_name(name);
  }
  for (const auto& entry : core.session().representations)
  {
    if (core.display_matches_source(entry.second,id,core.session().active_view_id))
    {
      return entry.second.vectors;
    }
  }
  error = "The source has no representation in the active view.";
  return std::nullopt;
}
bool PostprocessorApplicationInterface::set_vector_plot(const std::string& name,
  const Postprocessor::VectorPlotDefinition& value,std::string& error)
{
  std::string id = core.session().active_source_id;
  if (!name.empty())
  {
    id = core.source_id_from_name(name);
  }
  return core.set_vector_plot(id,core.session().active_view_id,value,error);
}
bool PostprocessorApplicationInterface::vector_field(const std::string& name,const std::string& field,
  std::optional<std::size_t> dataset,std::string& error,const Postprocessor::FieldQualifiers& qualifiers)
{
  auto settings = vector_plot(name,error);
  if (!settings)
  {
    return false;
  }
  std::string id = core.session().active_source_id;
  if (!name.empty())
  {
    id = core.source_id_from_name(name);
  }
  Postprocessor::ResultFieldGroup match;
  if (!Postprocessor::resolve_result_field(core.field_groups(id,std::nullopt),{field,qualifiers},dataset,
      {},true,true,match,error))
  {
    return false;
  }
  if (!match.unit_issue.empty())
  {
    error = match.unit_issue;
    return false;
  }
  settings->field = Postprocessor::FieldSelection{dataset,match.field.array_name,match.field.association,{},true,match.key};
  return set_vector_plot(name,*settings,error);
}
