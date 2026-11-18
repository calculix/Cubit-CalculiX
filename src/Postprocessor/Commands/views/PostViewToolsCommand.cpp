#include "PostViewToolsCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <cctype>
#include <utility>
PostViewToolsCommand::PostViewToolsCommand(std::weak_ptr<PostprocessorApplicationInterface> value) : application(std::move(value))
{
}
std::vector<std::string> PostViewToolsCommand::get_syntax()
{
  return {"post view {front|back|top|bottom|left|right|iso}",
    "post view projection {parallel|perspective}","post scale {on|off}",
    "post properties {legend|vectors|vectorlegend|clip|section|threshold|explosion|close}",
    "post properties legend source <string:type='quoted',number='1',label='source_name'>",
    "post properties vectorlegend source <string:type='quoted',number='1',label='source_name'>",
    "post properties vectors source <string:type='quoted',number='1',label='source_name'>",
    "post properties source <string:type='quoted',number='1',label='source_name'>"};
}
std::vector<std::string> PostViewToolsCommand::get_syntax_help()
{
  return get_syntax();
}
std::vector<std::string> PostViewToolsCommand::get_help()
{
  return {"Open a contextual properties editor, choose a standard Postprocessor view, or toggle the unit-aware scale. Camera commands affect only the Postprocessor viewport."};
}
bool PostViewToolsCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto app = application.lock();
    if (!app)
    {
      error = "The Postprocessor application is unavailable.";
    }
    else if (data.find_keyword("PROPERTIES"))
    {
      std::string page;
      std::string source;
      if (data.find_keyword("SOURCE"))
      {
        page = "source";
        data.get_string("source_name",source);
      }
      for (const auto* name : {"legend","vectors","vectorlegend","clip","section","threshold","explosion","close"})
      {
        std::string keyword(name);
        for (auto& character : keyword)
        {
          character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        }
        if (data.find_keyword(keyword.c_str()))
        {
          page = name;
        }
      }
      if (app->show_properties(page,source,error))
      {
        return true;
      }
    }
    else if (data.find_keyword("SCALE"))
    {
      if (app->set_scale_visible(data.find_keyword("ON"),error))
      {
        return true;
      }
    }
    else
    {
      for (const auto* name : {"front","back","top","bottom","left","right","iso","parallel","perspective"})
      {
        std::string keyword(name);
        for (auto& character : keyword)
        {
          character = static_cast<char>(std::toupper(static_cast<unsigned char>(character)));
        }
        if (data.find_keyword(keyword.c_str()) && app->standard_view(name,error))
        {
          return true;
        }
      }
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  if (error.empty())
  {
    error = "Invalid view tool command.";
  }
  PRINT_ERROR("Cannot change Postprocessor view tools: %s\n",error.c_str());
  return false;
}
