#include "PostTableCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
PostTableCommand::PostTableCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostTableCommand::get_syntax()
{
  return {"post table {open|field} <string:type='quoted',number='1',label='table_name'> "
    "[source <string:type='quoted',number='1',label='source_name'>]"};
}
std::vector<std::string> PostTableCommand::get_syntax_help()
{
  return {"post table open \"<table name or exact key>\" [source \"<source>\"]",
    "post table field \"<field key>\" [source \"<source>\"]"};
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
    if (!active->open_table(source,name,data.find_keyword("FIELD"),error))
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
