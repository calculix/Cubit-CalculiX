#include "PostViewTypeCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
PostViewTypeCommand::PostViewTypeCommand(std::weak_ptr<PostprocessorApplicationInterface> active) : application(std::move(active))
{
}
std::vector<std::string> PostViewTypeCommand::get_syntax()
{
  return {"post view type 3d"};
}
std::vector<std::string> PostViewTypeCommand::get_syntax_help()
{
  return get_syntax();
}
std::vector<std::string> PostViewTypeCommand::get_help()
{
  return {"Return the active table view to its retained Postprocessor 3D scene."};
}
bool PostViewTypeCommand::execute(CubitCommandData&)
{
  try
  {
    const auto active = application.lock();
    std::string error;
    if (!active || !active->show_3d(error))
    {
      PRINT_ERROR("Cannot show 3D view: %s\n",error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot show 3D view: %s\n",exception.what());
    return false;
  }
}
