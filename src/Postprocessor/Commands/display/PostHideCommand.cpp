#include "PostHideCommand.hpp"
#include "DatasetCommandSupport.hpp"
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
  auto result = DatasetCommandSupport::syntax(false);
  result.push_back("post hide <string:type='quoted',number='1',label='source_name',help='<source name>'>");
  return result;
}

std::vector<std::string> PostHideCommand::get_syntax_help()
{
  return {"post hide \"<source name>\"", "post hide dataset {ID|\"name\"} [source \"name\"]"};
}

std::vector<std::string> PostHideCommand::get_help()
{
  return {"Hide an opened source or dataset in the active view while retaining data and displays in other views.",
    "Dataset selectors accept an ID or a quoted unique published name. Source defaults to the active source.",
    "Hiding a source preserves its dataset choices. Hiding datasets does not unload their result arrays."};
}

bool PostHideCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, error;
    if (active && data.find_keyword("DATASET"))
    {
      if (!DatasetCommandSupport::execute(data,*active,false,error))
      {
        PRINT_ERROR("Cannot hide dataset: %s\n",error.c_str());
        return false;
      }
      return true;
    }
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
