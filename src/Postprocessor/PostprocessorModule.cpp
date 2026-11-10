#include "PostprocessorModule.hpp"
#include "Commands/PostprocessorCommands.hpp"
#include "Interfaces/PostprocessorApplicationInterface.hpp"
#include "GUI/PostprocessorGui.hpp"
#include "EventObservers/PostprocessorModelObserver.hpp"
#include "CubitMessage.hpp"
#include <utility>

PostprocessorModule::PostprocessorModule(ResultSourceRegistryInterface& providers,
  std::function<bool(const std::string&, std::string&)> submit,
  std::function<bool(const std::string&, std::string&)> silent)
  : providers(providers), submitCommand(std::move(submit)), silentCommand(std::move(silent))
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

  if (application != nullptr && commands != nullptr && modelObserver != nullptr)
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
  modelObserver = std::make_unique<PostprocessorModelObserver>([this]()
  {
    std::string error;
    if (!application->reset(error))
    {
      PRINT_ERROR("Cannot reset Postprocessor: %s\n", error.c_str());
      return;
    }
    if (gui != nullptr)
    {
      // Refresh synchronously to release actors and their retained snapshots,
      // including when another workspace is active. No nested command is sent.
      gui->refresh();
    }
  });
  return true;
}

void PostprocessorModule::shutdown() noexcept
{
  modelObserver.reset();
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
    gui = std::make_unique<PostprocessorGui>(application, submitCommand, silentCommand);
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

ProjectStateProviderInterface* PostprocessorModule::project_state_provider() noexcept
{
  return this;
}
std::string PostprocessorModule::project_state_id() const
{
  return "postprocessor";
}
bool PostprocessorModule::prepare_project_read(const ProjectStateContext& context, std::string& error)
{
  return application && application->read_project_definitions(context.filename,error);
}
bool PostprocessorModule::resolve_project_sources(const ProjectStateContext&, std::string& error)
{
  return application && application->resolve_project_sources(error);
}
bool PostprocessorModule::commit_project_read(const ProjectStateContext&, std::string& error)
{
  if (!application || !application->commit_project_restore(error))
  {
    return false;
  }
  if (gui)
  {
    gui->refresh();
  }
  return true;
}
void PostprocessorModule::discard_project_read() noexcept
{
  if (application)
  {
    application->discard_project_restore();
  }
}
bool PostprocessorModule::prepare_project_save(const ProjectStateContext&, std::string& error)
{
  return application && application->prepare_project_save(error);
}
bool PostprocessorModule::write_project_state(const ProjectStateContext& context, std::string& error)
{
  return application && application->write_project(context.filename,error);
}
