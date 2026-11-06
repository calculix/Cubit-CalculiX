#include "CubitSimPlugin.hpp"
#include "Framework/Commands/WorkspaceCommand.hpp"
#include "Framework/Commands/DocumentationCommand.hpp"
#include "Framework/Commands/CubitSimVersionCommand.hpp"
#include "Framework/Core/FrameworkContext.hpp"
#include "CubitMessage.hpp"
#include "CubitPluginExport.hpp"

CUBIT_PLUGIN(CubitSimPlugin)

CubitSimPlugin::CubitSimPlugin()
{
  std::string error;
  acquired = framework_context().acquire_command_plugin(error);

  if (!acquired)
  {
    PRINT_ERROR("cubit-sim framework initialization failed: %s\n", error.c_str());
  }
}

CubitSimPlugin::~CubitSimPlugin()
{
  if (acquired)
  {
    framework_context().release_command_plugin();
  }
}

std::vector<std::string> CubitSimPlugin::get_keys()
{
  // Version reporting remains available if workspace initialization fails.
  std::vector<std::string> keys = {"CubitSimVersionCommand"};
  if (!acquired)
  {
    return keys;
  }
  keys.push_back("CubitSimWorkspaceCommand");
  keys.push_back("CubitSimDocumentationCommand");
  const auto moduleKeys = framework_context().module_manager().command_keys();
  keys.insert(keys.end(), moduleKeys.begin(), moduleKeys.end());
  return keys;
}

CubitCommand* CubitSimPlugin::create_command(const std::string& key)
{
  if (key == "CubitSimVersionCommand")
  {
    return new CubitSimVersionCommand();
  }
  if (!acquired)
  {
    return nullptr;
  }
  if (key == "CubitSimDocumentationCommand")
  {
    return new DocumentationCommand();
  }
  if (key == "CubitSimWorkspaceCommand")
  {
    return new WorkspaceCommand();
  }
  return framework_context().module_manager().create_command(key);
}
