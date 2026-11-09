#include "PostFrameCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <exception>
#include <limits>
#include <utility>

PostFrameCommand::PostFrameCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostFrameCommand::get_syntax()
{
  return {"post frame <value:label='frame_number',help='<1-based frame number>'> "
    "[source <string:type='quoted',number='1',label='source_name',help='<source name>'>]"};
}

std::vector<std::string> PostFrameCommand::get_syntax_help()
{
  return {"post frame <number> [source \"<source name>\"]"};
}

std::vector<std::string> PostFrameCommand::get_help()
{
  return {"Select a 1-based result frame in the active view; source defaults to the active source.",
    "Frame numbers are positions in the loaded sequence, not solver increment identifiers.",
    "Preserves visibility and camera. Frames without displayable mesh are rejected."};
}

bool PostFrameCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    double number = 0;
    std::string name, error;
    if (!active || !data.get_value("frame_number", number) || !std::isfinite(number) ||
        number < 1 || number != std::floor(number) || number > std::numeric_limits<int>::max())
    {
      PRINT_ERROR("Frame selection requires an available application and a positive integer frame number.\n");
      return false;
    }
    if (data.find_keyword("SOURCE") && (!data.get_string("source_name", name) || name.empty()))
    {
      PRINT_ERROR("Frame source requires a nonempty quoted source name.\n");
      return false;
    }
    if (!active->set_frame(name, static_cast<std::size_t>(number), error))
    {
      PRINT_ERROR("Cannot select Postprocessor frame: %s\n", error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot select Postprocessor frame: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot select Postprocessor frame: unknown failure.\n");
    return false;
  }
}
