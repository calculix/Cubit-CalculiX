#include "PostProbeCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
PostProbeCommand::PostProbeCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostProbeCommand::get_syntax()
{
  return {"post probe"};
}
std::vector<std::string> PostProbeCommand::get_syntax_help()
{
  return get_syntax();
}
std::vector<std::string> PostProbeCommand::get_help()
{
  return {"Print the selected entity's identity, coordinates and current result samples. No averaging or extrapolation."};
}
bool PostProbeCommand::execute(CubitCommandData&)
{
  try
  {
    const auto active = application.lock();
    std::string error;
    const auto result = active ? active->probe(error) : std::nullopt;
    if (!result)
    {
      PRINT_ERROR("Cannot probe selection: %s\n",error.empty() ? "application unavailable" : error.c_str());
      return false;
    }
    const auto text = Postprocessor::probe_text(*result);
    PRINT_INFO("%s\n",text.c_str());
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot probe selection: %s\n",exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot probe selection: unknown failure.\n");
  }
  return false;
}
