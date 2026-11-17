#include <type_traits>
#include "PostTableCommand.hpp"
#include "TableCommandOptions.hpp"
#include "Postprocessor/Commands/FieldCommandOptions.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
PostTableCommand::PostTableCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostTableCommand::get_syntax()
{
  const std::string target = "<string:type='quoted',number='1',label='table_name'>";
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post table open "+target+Postprocessor::table_qualifier_syntax()+source,
    "post table field "+target+Postprocessor::field_qualifier_syntax()+source,
    "post table filter column <string:type='quoted',number='1',label='column'> equals <string:type='quoted',number='1',label='exact'> [unit <string:type='quoted',number='1',label='unit'>]",
    "post table filter column <string:type='quoted',number='1',label='column'> range <value:label='minimum'> <value:label='maximum'> [unit <string:type='quoted',number='1',label='unit'>]",
    "post table filter clear column <string:type='quoted',number='1',label='column'>",
    "post table filter clear all"};
}
std::vector<std::string> PostTableCommand::get_syntax_help()
{
  return {"post table open \"name\" [scope frame|global] [type \"metadata type\"] [entry N] [source \"source\"]",
    "post table field \"name\" [origin \"FRD\"] [association node|element|integrationpoint] [set \"name\"] [source \"source\"]",
    "post table filter column \"name\" equals \"value\" [unit \"unit\"]",
    "post table filter column \"name\" range MIN MAX [unit \"unit\"]",
    "post table filter clear column \"name\"","post table filter clear all"};
}
std::vector<std::string> PostTableCommand::get_help()
{
  return {"Open a native table or spatial field in the active view. Native tables preserve their columns; field tables expose original IDs and values. Column filters combine with AND without changing source row order."};
}
bool PostTableCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, source, error;
    if (active && data.find_keyword("FILTER"))
    {
      Postprocessor::TableFilter filter;
      data.get_string("column",filter.column);
      data.get_string("exact",filter.value);
      data.get_string("unit",filter.unit);
      std::string action = "set";
      if (data.find_keyword("CLEAR"))
      {
        action = "clear";
        if (data.find_keyword("ALL"))
        {
          action = "clearall";
        }
      }
      else if (data.find_keyword("RANGE"))
      {
        filter.mode = Postprocessor::TableFilterMode::Range;
        if (!data.get_value("minimum",filter.minimum) || !data.get_value("maximum",filter.maximum))
        {
          PRINT_ERROR("Two numeric range bounds are required.\n");
          return false;
        }
      }
      if (!active->change_table_filter(action,filter,error))
      {
        PRINT_ERROR("Cannot change table filter: %s\n",error.c_str());
        return false;
      }
      return true;
    }
    if (!active || !data.get_string("table_name",name) || name.empty() ||
        (data.find_keyword("SOURCE") && !data.get_string("source_name",source)))
    {
      PRINT_ERROR("A table command requires an application and a quoted table/field name.\n");
      return false;
    }
    const bool field = data.find_keyword("FIELD");
    Postprocessor::TableQualifiers tableQualifiers;
    Postprocessor::FieldQualifiers fieldQualifiers;
    if (!([&]() -> std::common_type_t<decltype(Postprocessor::read_field_qualifiers(data,fieldQualifiers,error)), decltype(Postprocessor::read_table_qualifiers(data,tableQualifiers,error))> {
      if (field) {
        return Postprocessor::read_field_qualifiers(data,fieldQualifiers,error);
      } else {
        return Postprocessor::read_table_qualifiers(data,tableQualifiers,error);
      }
    }()) ||
        !active->open_table(source,name,field,error,tableQualifiers,fieldQualifiers))
    {
      PRINT_ERROR("Cannot open result table: %s\n",error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot open result table: %s\n",exception.what());
    return false;
  }
}
