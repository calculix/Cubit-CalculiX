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
    "post table field "+target+Postprocessor::field_qualifier_syntax()+source};
}
std::vector<std::string> PostTableCommand::get_syntax_help()
{
  return {"post table open \"name\" [scope frame|global] [type \"metadata type\"] [entry N] [source \"<source>\"]",
    "post table field \"name\" [origin \"FRD\"] [association node|element|integrationpoint] [source \"<source>\"]"};
}
std::vector<std::string> PostTableCommand::get_help()
{
  return {"Open a native table or spatial field in the active view. Native tables preserve their columns; field tables expose original IDs and values."};
}
bool PostTableCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, source, error;
    if (!active || !data.get_string("table_name",name) || name.empty() ||
        (data.find_keyword("SOURCE") && !data.get_string("source_name",source)))
    {
      PRINT_ERROR("A table command requires an application and a quoted table/field name.\n");
      return false;
    }
    const bool field = data.find_keyword("FIELD");
    Postprocessor::TableQualifiers tableQualifiers;
    Postprocessor::FieldQualifiers fieldQualifiers;
    if (!(field ? Postprocessor::read_field_qualifiers(data,fieldQualifiers,error) :
          Postprocessor::read_table_qualifiers(data,tableQualifiers,error)) ||
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
