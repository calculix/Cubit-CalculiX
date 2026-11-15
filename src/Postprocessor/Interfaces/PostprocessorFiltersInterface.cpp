#include <type_traits>
#include "PostprocessorApplicationInterface.hpp"
#include <algorithm>
std::string PostprocessorApplicationInterface::output_source(const Postprocessor::OutputReference& input) const
{
  return core.output_source(input);
}
bool PostprocessorApplicationInterface::display_matches_source(const Postprocessor::Representation& display,
  const std::string& source,const std::string& view) const
{
  return core.display_matches_source(display,source,view);
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
