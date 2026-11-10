#include "PostShowCommand.hpp"
#include "DatasetCommandSupport.hpp"
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
  auto result = DatasetCommandSupport::syntax(true);
  result.push_back("post show <string:type='quoted',number='1',label='source_name',help='<source name>'>");
  result.push_back("post show only <string:type='quoted',number='1',label='source_name'>");
  return result;
}

std::vector<std::string> PostShowCommand::get_syntax_help()
{
  return {"post show [only] \"<source name>\"", "post show [only] dataset {ID|\"name\"} [source \"name\"]",
    "post show dataset all [source \"name\"]"};
}

std::vector<std::string> PostShowCommand::get_help()
{
  return {"Show an opened mesh source or dataset in the active view without moving its established camera.",
    "Dataset selectors accept an ID or a quoted unique published name. Source defaults to the active source.",
    "Source-only syntax hides other sources in the active view; dataset-only syntax isolates within its source.",
    "Only hides other datasets within that source. Dataset all restores all of its datasets.",
    "Showing a dataset also enables its parent source. Showing a source retains its dataset preferences."};
}

bool PostShowCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, error;
    if (active && (data.find_keyword("DATASET") || data.find_keyword("BLOCKS") || data.find_keyword("NODESETS") ||
        data.find_keyword("SIDESETS") || data.find_keyword("INTEGRATIONPOINTS")))
    {
      if (!DatasetCommandSupport::execute(data,*active,true,error))
      {
        PRINT_ERROR("Cannot show dataset: %s\n",error.c_str());
        return false;
      }
      return true;
    }
    if (!active || !data.get_string("source_name", name) || !(data.find_keyword("ONLY") ? active->show_only_source(name,error) : active->set_source_visible(name,true,error)))
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
