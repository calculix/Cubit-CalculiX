#include "ReportModule.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"
#include <utility>
#include "Interfaces/ReportApplicationInterface.hpp"
#include "Commands/ReportCommands.hpp"
#include "GUI/ReportGui.hpp"
#include "EventObservers/ReportModelObserver.hpp"

ReportModule::ReportModule(ReportProviderRegistryInterface& providers,
  std::function<bool(const std::string&, std::string&)> send,
  std::function<bool(const std::string&, std::string&)> quiet)
  : registry(providers), submit(std::move(send)), silent(std::move(quiet))
{
}

ReportModule::~ReportModule()
{
  shutdown();
}

std::string ReportModule::module_id() const
{
  return "report";
}

bool ReportModule::initialize(UserOptionsInterface& options, DrawingInterface&, std::string& error)
{
  componentDirectory = options.component_directory();
  if (!application)
  {
    application = std::make_shared<ReportApplicationInterface>(registry);
    commands = std::make_unique<ReportCommands>(application);
    modelObserver = std::make_unique<ReportModelObserver>(application);
  }
  error.clear();
  return true;
}

void ReportModule::shutdown() noexcept
{
  modelObserver.reset();
  stop_component();
  commands.reset();
  application.reset();
}

CommandProviderInterface* ReportModule::command_provider() noexcept
{
  return commands ? this : nullptr;
}

ComponentProviderInterface* ReportModule::component_provider() noexcept
{
  return this;
}

WorkspaceProviderInterface* ReportModule::workspace_provider() noexcept
{
  return gui ? this : nullptr;
}

std::vector<std::string> ReportModule::command_keys() const
{
  return commands ? commands->command_keys() : std::vector<std::string>{};
}

CubitCommand* ReportModule::create_command(const std::string& key)
{
  return commands ? commands->create_command(key) : nullptr;
}

bool ReportModule::start_component(int withGUI, QWidget* host, std::string& error)
{
  if (withGUI <= 0 || gui)
  {
    error.clear();
    return true;
  }
  auto candidate = std::make_unique<ReportGui>(application, submit, silent, componentDirectory);
  if (!candidate->start(host, error))
  {
    return false;
  }
  gui = std::move(candidate);
  return true;
}

void ReportModule::stop_component() noexcept
{
  gui.reset();
}

bool ReportModule::workspace_view(WorkspaceView& view, std::string& error)
{
  if (!gui)
  {
    error = "Report GUI is not loaded.";
    return false;
  }
  return gui->workspace_view(view, error);
}

ProjectStateProviderInterface* ReportModule::project_state_provider() noexcept
{
  return this;
}
std::string ReportModule::project_state_id() const
{
  return "report";
}
void ReportModule::set_project_coordinated(bool coordinated)
{
  if (modelObserver)
  {
    modelObserver->project_coordinated = coordinated;
  }
}
bool ReportModule::prepare_project_read(const ProjectStateContext& context, std::string& error)
{
  return application && application->prepare_project_read(context.filename,error);
}
bool ReportModule::commit_project_read(const ProjectStateContext&, std::string& error)
{
  return application && application->commit_project_read(error);
}
void ReportModule::discard_project_read() noexcept
{
  if (application)
  {
    application->discard_project_read();
  }
}
bool ReportModule::write_project_state(const ProjectStateContext& context, std::string& error)
{
  return application && application->write_project(context.filename,error);
}
