#include "PostRefreshCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"

#include <exception>
#include <utility>

PostRefreshCommand::PostRefreshCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostRefreshCommand::get_syntax()
{
  return {"post refresh <string:type='quoted',number='1',label='source_name',help='<source name>'>"};
}

std::vector<std::string> PostRefreshCommand::get_syntax_help()
{
  return {"post refresh \"<source name>\""};
}

std::vector<std::string> PostRefreshCommand::get_help()
{
  return {"Reload an opened source by its exact, case-sensitive name. Does not change views."};
}

bool PostRefreshCommand::execute(CubitCommandData& data)
{
  const auto active = application.lock();

  if (!active)
  {
    PRINT_ERROR("The Postprocessor application is unavailable.\n");
    return false;
  }

  try
  {
    std::string name;

    if (!data.get_string("source_name", name))
    {
      PRINT_ERROR("Postprocessor refresh requires a quoted source name.\n");
      return false;
    }

    std::string error;

    if (!active->refresh_source(name, error))
    {
      PRINT_ERROR("Cannot refresh Postprocessor source: %s\n", error.c_str());
      return false;
    }

    PRINT_INFO("Refreshed Postprocessor source \"%s\".\n", name.c_str());
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Postprocessor refresh failed: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor refresh failed with an unknown exception.\n");
    return false;
  }
}
