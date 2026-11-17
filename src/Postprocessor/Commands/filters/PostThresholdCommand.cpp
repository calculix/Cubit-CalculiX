#include "PostThresholdCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/FieldCommandOptions.hpp"
#include "CubitMessage.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <utility>
PostThresholdCommand::PostThresholdCommand(std::weak_ptr<PostprocessorApplicationInterface> value)
  : application(std::move(value))
{
}
std::vector<std::string> PostThresholdCommand::get_syntax()
{
  const std::string name = " <string:type='quoted',number='1',label='filter_name'>";
  const std::string field = " field <string:type='quoted',number='1',label='field_name'>"+
    Postprocessor::field_qualifier_syntax()+" component ";
  const std::string dataset = " [dataset <value:label='dataset_id'>]";
  std::vector<std::string> result;
  for (const auto& lower : {std::string("off"),std::string("<value:label='lower_value'>")})
  {
    for (const auto& upper : {std::string("off"),std::string("<value:label='upper_value'>")})
    {
      const std::string limits = " lower "+lower+" upper "+upper;
      for (const auto& component : {std::string("magnitude"),std::string("<string:type='quoted',number='1',label='component_name'>")})
      {
        result.push_back("post threshold create"+name+" [source <string:type='quoted',number='1',label='source_name'>]"+
          field+component+dataset+limits+" [nodes {all|any}]");
        result.push_back("post threshold configure"+name+field+component+dataset+limits+" nodes {all|any}");
      }
      result.push_back("post threshold limits"+name+limits);
    }
  }
  for (const auto& component : {std::string("magnitude"),std::string("<string:type='quoted',number='1',label='component_name'>")})
  {
    result.push_back("post threshold field"+name+field+component+dataset);
  }
  result.push_back("post threshold nodes"+name+" {all|any}");
  result.push_back("post threshold reference"+name+" {on|off}");
  return result;
}
std::vector<std::string> PostThresholdCommand::get_syntax_help()
{
  return {"post threshold create \"name\" [source \"source\"] field \"field\" [origin \"FRD\"] [association node|element|integrationpoint] [set \"set\"] component magnitude|\"component\" [dataset N] lower VALUE|off upper VALUE|off [nodes all|any]",
    "post threshold configure \"name\" field ... component ... lower VALUE|off upper VALUE|off nodes all|any",
    "post threshold field \"name\" field ... component ... [dataset N]",
    "post threshold limits \"name\" lower VALUE|off upper VALUE|off",
    "post threshold nodes \"name\" all|any; post threshold reference \"name\" on|off"};
}
std::vector<std::string> PostThresholdCommand::get_help()
{
  return {"Retain whole elements with inclusive scalar limits. All nodes is the default; any node is also available.",
    "Limits use the field's current Postprocessor presentation units. Invalid samples never match. Integration points remain discrete.",
    "Use post filter for selection, show/hide, enable/disable, rename and removal."};
}
bool PostThresholdCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto app = application.lock();
    std::string name, source;
    if (!app || !data.get_string("filter_name",name))
    {
      PRINT_ERROR("Threshold requires an available application and filter name.\n");
      return false;
    }
    const bool create = data.find_keyword("CREATE");
    const bool configure = data.find_keyword("CONFIGURE");
    Postprocessor::ThresholdDefinition requested;
    if (!create)
    {
      const auto& filters = app->session().filters;
      const auto found = std::find_if(filters.begin(),filters.end(),[&](const auto& entry)
      {
        return entry.second.name == name && entry.second.kind == Postprocessor::FilterKind::Threshold;
      });
      if (found == filters.end())
      {
        PRINT_ERROR("No threshold filter has that name.\n");
        return false;
      }
      requested = found->second.threshold;
      source = app->session().sources.at(found->second.input.object_id).name;
    }
    else
    {
      data.get_string("source_name",source);
    }
    bool accepted = false;
    if (data.find_keyword("REFERENCE"))
    {
      std::string value = "off";
      if (data.find_keyword("ON"))
      {
        value = "on";
      }
      accepted = app->change_filter(name,"reference","",{},value,error);
    }
    else
    {
      const bool fieldChange = data.find_keyword("FIELD");
      if (fieldChange)
      {
        std::string field, component;
        std::optional<std::size_t> dataset;
        double index = 0;
        Postprocessor::FieldQualifiers qualifiers;
        const bool magnitude = data.find_keyword("MAGNITUDE");
        // An explicit field command addresses the current dataset IDs.
        requested.dataset_kind.clear();
        requested.dataset_name.clear();
        requested.dataset_model = -1;
        if (data.find_keyword("DATASET"))
        {
          if (!data.get_value("dataset_id",index) || !std::isfinite(index) || index < 0 ||
              index != std::floor(index) || index > 2147483647)
          {
            PRINT_ERROR("Threshold dataset must be an integer in [0,2147483647].\n");
            return false;
          }
          dataset = static_cast<std::size_t>(index);
        }
        if (!data.get_string("field_name",field) || (!magnitude && !data.get_string("component_name",component)) ||
            !Postprocessor::read_field_qualifiers(data,qualifiers,error) ||
            !app->threshold_field(source,{field,qualifiers},component,magnitude,dataset,requested,error))
        {
          PRINT_ERROR("Cannot select threshold field: %s\n",error.c_str());
          return false;
        }
      }
      if (create || configure || data.find_keyword("LIMITS"))
      {
        // Rebind numbers to the current presentation unit even for limits-only edits.
        if (!fieldChange)
        {
          Postprocessor::ResultFieldSelector selector{requested.field.group_key,{}};
          if (selector.name.empty())
          {
            selector.name = requested.field.array_name;
          }
          if (!app->threshold_field(source,selector,requested.field.component,requested.field.magnitude,
              requested.field.dataset_id,requested,error))
          {
            PRINT_ERROR("Cannot bind threshold limits: %s\n",error.c_str());
            return false;
          }
        }
        double value = 0;
        requested.lower.reset();
        requested.upper.reset();
        if (data.get_value("lower_value",value))
        {
          requested.lower = value;
        }
        if (data.get_value("upper_value",value))
        {
          requested.upper = value;
        }
      }
      if (data.find_keyword("NODES"))
      {
        requested.node_rule = Postprocessor::ThresholdNodeRule::All;
        if (data.find_keyword("ANY"))
        {
          requested.node_rule = Postprocessor::ThresholdNodeRule::Any;
        }
      }
      if (create)
      {
        Postprocessor::FilterDefinition filter;
        filter.name = name;
        filter.kind = Postprocessor::FilterKind::Threshold;
        filter.threshold = requested;
        accepted = app->create_filter(filter,source,error);
      }
      else
      {
        accepted = app->change_threshold(name,requested,error);
      }
    }
    if (accepted)
    {
      return true;
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  PRINT_ERROR("Cannot change threshold: %s\n",error.c_str());
  return false;
}
