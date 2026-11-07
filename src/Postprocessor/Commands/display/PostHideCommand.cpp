#include "PostHideCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostHideCommand::PostHideCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostHideCommand::get_syntax()
{
  return {"post hide <string:type='quoted',number='1',label='source_name',help='<source name>'>"};
}

std::vector<std::string> PostHideCommand::get_syntax_help()
{
  return {"post hide \"<source name>\""};
}

std::vector<std::string> PostHideCommand::get_help()
{
  return {"Hide an opened source in the active Postprocessor view while retaining its data and displays in other views."};
}

bool PostHideCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, error;
    if (!active || !data.get_string("source_name", name) || !active->set_source_visible(name, false, error))
    {
      PRINT_ERROR("Cannot hide Postprocessor source: %s\n", error.empty() ? "application unavailable or missing quoted source name" : error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot hide Postprocessor source: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor display command failed with an unknown exception.\n");
    return false;
  }
}
