#include "FEAPlugin.hpp"
#include "Framework/Commands/WorkspaceCommand.hpp"
#include "Framework/Core/FrameworkContext.hpp"
#include "CubitMessage.hpp"
#include "CubitPluginExport.hpp"

CUBIT_PLUGIN(FEAPlugin)

FEAPlugin::FEAPlugin()
{
  std::string error;
  acquired = framework_context().acquire_command_plugin(error);

  if (!acquired)
  {
    PRINT_ERROR("FEA framework initialization failed: %s\n", error.c_str());
  }
}

FEAPlugin::~FEAPlugin()
{
  if (acquired)
  {
    framework_context().release_command_plugin();
  }
}

std::vector<std::string> FEAPlugin::get_keys()
{
  if (!acquired)
  {
    return {};
  }

  auto keys = framework_context().module_manager().command_keys();
  keys.insert(keys.begin(), "feaWorkspaceCommand");
  return keys;
}

CubitCommand* FEAPlugin::create_command(const std::string& key)
{
  if (!acquired)
  {
    return nullptr;
  }

  if (key == "feaWorkspaceCommand")
  {
    return new WorkspaceCommand();
  }

  return framework_context().module_manager().create_command(key);
}
