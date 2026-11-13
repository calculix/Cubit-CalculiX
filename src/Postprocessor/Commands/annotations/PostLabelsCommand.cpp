#include "PostLabelsCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
PostLabelsCommand::PostLabelsCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostLabelsCommand::get_syntax()
{
  const std::string view = " [view <string:type='quoted',number='1',label='view_id'>]";
  return {"post labels extrema {on|off}"+view,"post labels {show|hide}"+view};
}
std::vector<std::string> PostLabelsCommand::get_syntax_help()
{
  return {"post labels extrema {on|off} [view \"id\"]","post labels {show|hide} [view \"id\"]"};
}
std::vector<std::string> PostLabelsCommand::get_help()
{
  return {"Show current-frame extrema over visible datasets, or toggle all labels without deleting their settings."};
}
bool PostLabelsCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string view, error;
    data.get_string("view_id",view);
    if (active && active->set_labels(view,data.find_keyword("EXTREMA"),
        data.find_keyword("ON") || data.find_keyword("SHOW"),error))
    {
      return true;
    }
    PRINT_ERROR("Cannot change result labels: %s\n",error.empty() ? "Annotations are unavailable." : error.c_str());
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change result labels: %s\n",exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot change result labels: unknown failure.\n");
  }
  return false;
}
