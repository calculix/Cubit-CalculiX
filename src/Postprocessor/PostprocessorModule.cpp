#include "PostprocessorModule.hpp"
#include "Commands/PostprocessorCommands.hpp"
#include "Interfaces/PostprocessorApplicationInterface.hpp"
#include "GUI/PostprocessorGui.hpp"
#include <utility>

PostprocessorModule::PostprocessorModule(ResultSourceRegistryInterface& providers,
  std::function<bool(const std::string&, std::string&)> submit)
  : providers(providers), submitCommand(std::move(submit))
{
}

PostprocessorModule::~PostprocessorModule()
{
  shutdown();
}

std::string PostprocessorModule::module_id() const
{
  return "postprocessor";
}

bool PostprocessorModule::initialize(UserOptionsInterface& options, DrawingInterface&, std::string& error)
{
  error.clear();

  if (application != nullptr && commands != nullptr)
  {
    return true;
  }

  auto candidate = std::make_shared<PostprocessorApplicationInterface>(providers);
  if (!candidate->initialize_user_options(options, error))
  {
    return false;
  }
  application = std::move(candidate);
  commands = std::make_unique<PostprocessorCommands>(application);
  return true;
}

void PostprocessorModule::shutdown() noexcept
{
  stop_component();
  commands.reset();
  if (application != nullptr)
  {
    application->shutdown_user_options();
  }
  application.reset();
}

CommandProviderInterface* PostprocessorModule::command_provider() noexcept
{
  return commands == nullptr ? nullptr : this;
}

ComponentProviderInterface* PostprocessorModule::component_provider() noexcept
{
  return this;
}

std::vector<std::string> PostprocessorModule::command_keys() const
{
  return commands == nullptr ? std::vector<std::string>{} : commands->command_keys();
}

CubitCommand* PostprocessorModule::create_command(const std::string& key)
{
  return commands == nullptr ? nullptr : commands->create_command(key);
}

bool PostprocessorModule::start_component(int withGUI, QWidget* host, std::string& error)
{
  error.clear();

  if (withGUI <= 0)
  {
    return true;
  }

  if (gui == nullptr)
  {
    gui = std::make_unique<PostprocessorGui>(application, submitCommand);
  }

  return gui->start(host, error);
}

void PostprocessorModule::stop_component() noexcept
{
  gui.reset();
}

WorkspaceProviderInterface* PostprocessorModule::workspace_provider() noexcept
{
  return gui == nullptr ? nullptr : this;
}

bool PostprocessorModule::workspace_view(WorkspaceView& view, std::string& error)
{
  if (gui == nullptr)
  {
    error = "The Postprocessor GUI component is not loaded.";
    return false;
  }

  return gui->workspace_view(view, error);
}
