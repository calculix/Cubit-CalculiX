#include "PostOpenCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include "CubitInterface.hpp"

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
    "[name <string:type='quoted',number='1',label='source_name',help='<source name>'>]",
    "post open ccx file <string:type='quoted',number='1',label='file_path'> [dat <string:type='quoted',number='1',label='dat_path'>] [input <string:type='quoted',number='1',label='input_path'>] [name <string:type='quoted',number='1',label='source_name'>]",
    "post open file <string:type='quoted',number='1',label='file_path'> [name <string:type='quoted',number='1',label='source_name'>]"};
}

std::vector<std::string> PostOpenCommand::get_syntax_help()
{
  return {"post open <provider> job <job_id> [name \"<source name>\"]",
    "post open file \"<path.vtu|vtpc|vtt>\" [name \"<source name>\"]"};
}

std::vector<std::string> PostOpenCommand::get_help()
{
  return {"Load job results through a registered provider, for example: post open ccx job 12.",
    "Standalone VTK files use: post open file \"C:/results/mesh.vtu\".",
    "External CalculiX results use: post open ccx file \"beam.frd\" [dat \"beam.dat\"] [input \"beam.inp\"].",
    "The source takes the job name unless a quoted name is supplied. Names must be unique.",
    "Selects the opened source and switches to the Postprocessor workspace after opening succeeds."};
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

    const bool file = data.find_keyword("FILE");
    std::string path;
    if (file ? !data.get_string("file_path",path) : (!data.get_string("provider", prefix) || !data.get_value("job_id", job_id)))
    {
      PRINT_ERROR("Postprocessor opening requires a provider/job or a quoted result file path.\n");
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

    std::vector<std::string> paths;
    if (file)
    {
      paths.push_back(path);
      for (const auto& option : {std::pair<const char*,const char*>{"DAT","dat_path"},{"INPUT","input_path"}})
      {
        if (data.find_keyword(option.first))
        {
          std::string companion;
          if (!data.get_string(option.second,companion) || companion.empty())
          {
            PRINT_ERROR("Result companion paths must be nonempty quoted strings.\n");
            return false;
          }
          paths.push_back(std::move(companion));
        }
      }
    }
    if (!(file ? active->open_file(data.find_keyword("CCX") ? "ccx" : "vtk",paths,name,error) :
        active->open_job(prefix,job_id,name,error)))
    {
      PRINT_ERROR("Cannot open Postprocessor source: %s\n", error.c_str());
      return false;
    }

    const auto& session = active->session();
    const auto& source = session.sources.at(session.active_source_id);
    PRINT_INFO("Opened Postprocessor source \"%s\".\n", source.name.c_str());
    if (!CubitInterface::cmd_single("sim workspace postprocessor"))
    {
      PRINT_ERROR("Results are open, but switching to the Postprocessor workspace failed.\n");
      return false;
    }
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
