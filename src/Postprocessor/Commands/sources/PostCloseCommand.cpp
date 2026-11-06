#include "PostCloseCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"

#include <exception>
#include <utility>

PostCloseCommand::PostCloseCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostCloseCommand::get_syntax()
{
  return {"post close <string:type='quoted',number='1',label='source_name',help='<source name>'>",
    "post close all"};
}

std::vector<std::string> PostCloseCommand::get_syntax_help()
{
  return {"post close \"<source name>\"", "post close all"};
}

std::vector<std::string> PostCloseCommand::get_help()
{
  return {"Close a named source, or all sources, without deleting solver jobs or files.",
    "A quoted name \"all\" addresses that source; the unquoted keyword all closes every source.",
    "Views and layout are retained."};
}

bool PostCloseCommand::execute(CubitCommandData& data)
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
    std::string error;

    // Resolve the quoted-string form first, keeping a source named "all"
    // distinct from the unquoted bulk-close syntax.
    if (data.get_string("source_name", name))
    {
      if (!active->close_source(name, error))
      {
        PRINT_ERROR("Cannot close Postprocessor source: %s\n", error.c_str());
        return false;
      }

      PRINT_INFO("Closed Postprocessor source \"%s\".\n", name.c_str());
      return true;
    }

    if (!data.find_keyword("ALL"))
    {
      PRINT_ERROR("Postprocessor closure requires a quoted source name or all.\n");
      return false;
    }

    if (!active->close_all_sources(error))
    {
      PRINT_ERROR("Cannot close Postprocessor sources: %s\n", error.c_str());
      return false;
    }

    PRINT_INFO("Closed all Postprocessor sources.\n");
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Postprocessor closure failed: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor closure failed with an unknown exception.\n");
    return false;
  }
}
