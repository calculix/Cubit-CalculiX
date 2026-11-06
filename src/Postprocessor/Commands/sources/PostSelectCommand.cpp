#include "PostSelectCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"

#include <exception>
#include <utility>

PostSelectCommand::PostSelectCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostSelectCommand::get_syntax()
{
  return {"post select <string:type='quoted',number='1',label='source_name',help='<source name>'>"};
}

std::vector<std::string> PostSelectCommand::get_syntax_help()
{
  return {"post select \"<source name>\""};
}

std::vector<std::string> PostSelectCommand::get_help()
{
  return {"Select an opened source by its exact, case-sensitive name. Does not change views."};
}

bool PostSelectCommand::execute(CubitCommandData& data)
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
      PRINT_ERROR("Postprocessor selection requires a quoted source name.\n");
      return false;
    }

    std::string error;

    if (!active->select_source(name, error))
    {
      PRINT_ERROR("Cannot select Postprocessor source: %s\n", error.c_str());
      return false;
    }

    PRINT_INFO("Selected Postprocessor source \"%s\".\n", name.c_str());
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Postprocessor selection failed: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor selection failed with an unknown exception.\n");
    return false;
  }
}
