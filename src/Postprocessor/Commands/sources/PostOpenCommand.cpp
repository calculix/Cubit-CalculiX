#include "PostOpenCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"

#include <exception>
#include <optional>
#include <utility>

PostOpenCommand::PostOpenCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostOpenCommand::get_syntax()
{
  return {"post open <string:type='unquoted',number='1',label='provider',help='<provider prefix>'> "
    "job <value:label='job_id',help='<job ID>'> "
    "[name <string:type='quoted',number='1',label='source_name',help='<source name>'>]"};
}

std::vector<std::string> PostOpenCommand::get_syntax_help()
{
  return {"post open <provider> job <job_id> [name \"<source name>\"]"};
}

std::vector<std::string> PostOpenCommand::get_help()
{
  return {"Load job results through a registered provider, for example: post open ccx job 12.",
    "The source takes the job name unless a quoted name is supplied. Names must be unique.",
    "Selects the opened source. Does not switch workspace, create a display, or split a view."};
}

bool PostOpenCommand::execute(CubitCommandData& data)
{
  const auto active = application.lock();

  if (!active)
  {
    PRINT_ERROR("The Postprocessor application is unavailable.\n");
    return false;
  }

  try
  {
    std::string prefix;
    int job_id = -1;

    if (!data.get_string("provider", prefix) || !data.get_value("job_id", job_id))
    {
      PRINT_ERROR("Postprocessor opening requires a provider prefix and job ID.\n");
      return false;
    }

    std::optional<std::string> name;

    if (data.find_keyword("NAME"))
    {
      std::string supplied;

      if (!data.get_string("source_name", supplied))
      {
        PRINT_ERROR("Postprocessor source name must be a quoted string.\n");
        return false;
      }

      name = std::move(supplied);
    }

    std::string error;

    if (!active->open_job(prefix, job_id, name, error))
    {
      PRINT_ERROR("Cannot open Postprocessor source: %s\n", error.c_str());
      return false;
    }

    const auto& session = active->session();
    const auto& source = session.sources.at(session.active_source_id);
    PRINT_INFO("Opened Postprocessor source \"%s\".\n", source.name.c_str());
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Postprocessor open failed: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Postprocessor open failed with an unknown exception.\n");
    return false;
  }
}
