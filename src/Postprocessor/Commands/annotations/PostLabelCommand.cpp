#include "PostLabelCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <charconv>
#include <cmath>
#include <exception>
#include <utility>

PostLabelCommand::PostLabelCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostLabelCommand::get_syntax()
{
  const std::string name = "<string:type='quoted',number='1',label='label_name'>";
  const std::string id = "{<value:label='entity_id'>|<string:type='quoted',number='1',label='entity_text'>}";
  const std::string view = " [view <string:type='quoted',number='1',label='view_id'>]";
  const std::string tail = " [dataset {<value:label='dataset_id'>|<string:type='quoted',number='1',label='dataset_name'>}]"
    " [source <string:type='quoted',number='1',label='source_name'>]"
    " [field <string:type='quoted',number='1',label='field_name'>]"
    " [component {magnitude|<string:type='quoted',number='1',label='component_name'>}]"+view;
  return {"post label add ["+name+"] node "+id+tail,
    "post label add ["+name+"] element "+id+tail,
    "post label add ["+name+"] integrationpoint <value:label='ip'> element "+id+tail,
    "post label rename "+name+" <string:type='quoted',number='1',label='replacement'>"+view,
    "post label position "+name+" <value:label='x'> <value:label='y'>"+view,
    "post label {show|hide|remove} "+name+view};
}
std::vector<std::string> PostLabelCommand::get_syntax_help()
{
  return {"post label add [\"name\"] {node ID|element ID|integrationpoint N element ID} [dataset ID|\"name\"] [source \"name\"] [field \"key\"] [component magnitude|\"name\"] [view \"id\"]",
    "post label rename \"name\" \"replacement\"", "post label position \"name\" X Y",
    "post label {show|hide|remove} \"name\""};
}
std::vector<std::string> PostLabelCommand::get_help()
{
  return {"Labels bind original entities and fixed fields; omitted names use the next unused label_N.",
    "Positions are normalized bottom-left viewport coordinates in [0,1]. Quote IDs above 9007199254740991.",
    "@minimum and @maximum are reserved draggable extrema labels. All mutations support an optional view ID."};
}
bool PostLabelCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    if (!active)
    {
      PRINT_ERROR("Postprocessor annotations are unavailable.\n");
      return false;
    }
    std::string name, view, error, replacement;
    data.get_string("label_name",name);
    data.get_string("view_id",view);
    bool accepted = false;
    if (data.find_keyword("ADD"))
    {
      std::int64_t id = -1;
      std::string exact;
      double number = 0;
      if (data.get_string("entity_text",exact))
      {
        const auto result = std::from_chars(exact.data(),exact.data()+exact.size(),id);
        if (result.ec != std::errc{} || result.ptr != exact.data()+exact.size() || id < 0)
        {
          PRINT_ERROR("A label requires an exact nonnegative entity ID.\n");
          return false;
        }
      }
      else if (data.get_value("entity_id",number) && std::isfinite(number) && number >= 0 &&
          number <= 9007199254740991.0 && number == std::floor(number))
      {
        id = static_cast<std::int64_t>(number);
      }
      else
      {
        PRINT_ERROR("Invalid label entity ID; quote large IDs.\n");
        return false;
      }
      const auto kind = data.find_keyword("INTEGRATIONPOINT") ? Postprocessor::EntityKind::IntegrationPoint :
        data.find_keyword("NODE") ? Postprocessor::EntityKind::Node : Postprocessor::EntityKind::Element;
      std::optional<std::int64_t> ip;
      if (kind == Postprocessor::EntityKind::IntegrationPoint)
      {
        if (!data.get_value("ip",number) || !std::isfinite(number) || number < 1 || number > 2147483647 || number != std::floor(number))
        {
          PRINT_ERROR("Integration point requires a positive integer.\n");
          return false;
        }
        ip = static_cast<std::int64_t>(number);
      }
      std::optional<PostprocessorApplicationInterface::DatasetSelector> dataset;
      std::string datasetName, source, field, component, assigned;
      if (data.get_string("dataset_name",datasetName))
      {
        dataset = datasetName;
      }
      else if (data.find_keyword("DATASET"))
      {
        if (!data.get_value("dataset_id",number) || !std::isfinite(number) || number < 0 || number > 2147483647 || number != std::floor(number))
        {
          PRINT_ERROR("Invalid label dataset ID.\n");
          return false;
        }
        dataset = static_cast<std::size_t>(number);
      }
      data.get_string("source_name",source);
      data.get_string("field_name",field);
      data.get_string("component_name",component);
      const bool magnitude = data.find_keyword("MAGNITUDE");
      if (magnitude)
      {
        component = "magnitude";
      }
      accepted = active->add_label(view,name,source,dataset,kind,id,ip,field,component,magnitude,assigned,error);
    }
    else
    {
      std::array<double,2> position = Postprocessor::AnnotationDefaults::position;
      const std::string action = data.find_keyword("POSITION") ? "position" : data.find_keyword("RENAME") ? "rename" :
        data.find_keyword("SHOW") ? "show" : data.find_keyword("HIDE") ? "hide" : "remove";
      if (action == "position" && (!data.get_value("x",position[0]) || !data.get_value("y",position[1])))
      {
        return false;
      }
      data.get_string("replacement",replacement);
      accepted = active->change_label(view,name,action,replacement,position,error);
    }
    if (!accepted)
    {
      PRINT_ERROR("Cannot change result label: %s\n",error.c_str());
    }
    return accepted;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change result label: %s\n",exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot change result label: unknown failure.\n");
  }
  return false;
}
