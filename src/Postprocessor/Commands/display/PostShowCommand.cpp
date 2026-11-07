#include "PostShowCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>

PostShowCommand::PostShowCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostShowCommand::get_syntax()
{
  return {"post show <string:type='quoted',number='1',label='source_name',help='<source name>'>"};
}

std::vector<std::string> PostShowCommand::get_syntax_help()
{
  return {"post show \"<source name>\""};
}

std::vector<std::string> PostShowCommand::get_help()
{
  return {"Show an opened mesh source in the active Postprocessor view without changing selection or an established camera."};
}

bool PostShowCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, error;
    if (!active || !data.get_string("source_name", name) || !active->set_source_visible(name, true, error))
    {
      PRINT_ERROR("Cannot show Postprocessor source: %s\n", error.empty() ? "application unavailable or missing quoted source name" : error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot show Postprocessor source: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor display command failed with an unknown exception.\n");
    return false;
  }
}
