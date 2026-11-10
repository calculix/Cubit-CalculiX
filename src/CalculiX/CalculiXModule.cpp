#include "CalculiXModule.hpp"
#include "Core/reporting/CalculiXReportTemplateProvider.hpp"
#include "Core/reporting/CalculiXReportModelProvider.hpp"
#include "Core/jobs/CalculiXJobMonitorProvider.hpp"
#include "Commands/CalculiXCommands.hpp"
#include "GUI/CalculiXGui.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/application/CoreObserver.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "Core/results/CalculiXResultSourceProvider.hpp"
#include "CubitInterface.hpp"
#include "CubitCommandInterface.hpp"
#include "CubitPluginManager.hpp"

#include <algorithm>

#include <QPlainTextEdit>
#include <QTextCursor>

namespace
{
bool removeStartupText(QWidget* host, const QString& text)
{
  if (host == nullptr || text.isEmpty())
  {
    return false;
  }

  auto* window = host->findChild<QPlainTextEdit*>("mCommandWindow");

  if (window == nullptr)
  {
    return false;
  }

  const auto position = window->toPlainText().lastIndexOf(text);

  if (position < 0)
  {
    return false;
  }

  QTextCursor cursor(window->document());
  cursor.setPosition(static_cast<int>(position));
  cursor.setPosition(static_cast<int>(position + text.size()), QTextCursor::KeepAnchor);
  cursor.removeSelectedText();

  return true;
}
}

CalculiXModule::CalculiXModule() = default;

CalculiXModule::~CalculiXModule()
{
  shutdown();
}

std::string CalculiXModule::module_id() const
{
  return "calculix";
}

bool CalculiXModule::initialize(UserOptionsInterface& options, DrawingInterface& drawings, std::string& error)
{
  error.clear();

  if (commands != nullptr)
  {
    return true;
  }

  // Do not run the old and new command entry points in the same Cubit session.
  if (auto* manager = CubitInterface::plugin_manager())
  {
    for (auto* provider : manager->get_typed_interfaces<CubitCommandInterface*>())
    {
      const auto keys = provider->get_keys();

      if (std::find(keys.begin(), keys.end(), "ccxResetCommand") != keys.end())
      {
        error = "CalculiX commands are already registered. Restart Cubit using only the cubit-sim plugin directory.";
        return false;
      }
    }
  }

  CalculiXApplicationInterface application;
  if (!application.initialize_user_options(options, error))
  {
    return false;
  }

  if (!application.initialize_application())
  {
    error = "Cannot initialize the CalculiX core and Cubit export/material interfaces.";
    return false;
  }

  CalculiXDrawInterface().bind_drawing(&drawings);
  commands = std::make_unique<CalculiXCommands>();
  resultProvider = std::make_unique<CalculiXResultSourceProvider>();
  monitorProvider = std::make_unique<CalculiXJobMonitorProvider>();
  reportTemplates = std::make_unique<CalculiXReportTemplateProvider>();
  reportModels = std::make_unique<CalculiXReportModelProvider>();
  application.core_observer().register_observer();
  observer_registered = true;
  return true;
}

void CalculiXModule::shutdown() noexcept
{
  stop_component();
  reportModels.reset();
  reportTemplates.reset();
  monitorProvider.reset();
  resultProvider.reset();

  if (observer_registered)
  {
    CalculiXApplicationInterface application;
    application.core_observer().unregister_observer();
    observer_registered = false;
  }

  CalculiXDrawInterface().bind_drawing(nullptr);
  commands.reset();
  CalculiXApplicationInterface().shutdown_user_options();
  // Project data remains owned by the one ccx_core in calculix_core.
  // GUI detach does not reset the model or terminate running solver jobs.
}

CommandProviderInterface* CalculiXModule::command_provider() noexcept
{
  return this;
}

JobMonitorProviderInterface* CalculiXModule::job_monitor_provider() noexcept
{
  return monitorProvider.get();
}

ComponentProviderInterface* CalculiXModule::component_provider() noexcept
{
  return this;
}

std::vector<std::string> CalculiXModule::command_keys() const
{
  if (commands == nullptr)
  {
    return {};
  }

  return commands->command_keys();
}

CubitCommand* CalculiXModule::create_command(const std::string& key)
{
  if (commands == nullptr)
  {
    return nullptr;
  }

  return commands->create_command(key);
}

bool CalculiXModule::start_component(int withGUI, QWidget* host, std::string& error)
{
  error.clear();

  if (withGUI > 0)
  {
    removeStartupText(host, QStringLiteral(
      "ERROR: No element blocks have been established!\n"
      "Use the block command to assign the meshed geometry to an element block.\n"
      "Or, check that the default block type is correct. Current setting: All\n"
      "ERROR: Unable to find entities to write."));
  }

  CalculiXApplicationInterface application;

  if (!application.init_materiallibrary())
  {
    error = "Cannot initialize the CalculiX material library. Check its configured path.";
    return false;
  }

  // The SWIG component entry point releases the GIL before this SDK call.
  // Python and material-library initialization apply to headless sessions too.
  if (!application.init_pythoninterface())
  {
    error = "Cannot initialize the CalculiX Python interface.";
    return false;
  }

  if (withGUI > 0)
  {
    if (gui == nullptr)
    {
      gui = std::make_unique<CalculiXGui>();
    }

    return gui->start(host, error);
  }

  return true;
}

void CalculiXModule::stop_component() noexcept
{
  if (gui != nullptr)
  {
    gui->stop();
    gui.reset();
  }
}

ResultSourceProviderInterface* CalculiXModule::result_source_provider() noexcept
{
  return resultProvider.get();
}

WorkspaceProviderInterface* CalculiXModule::workspace_provider() noexcept
{
  return gui == nullptr ? nullptr : this;
}

bool CalculiXModule::workspace_view(WorkspaceView& view, std::string& error)
{
  if (gui == nullptr)
  {
    error = "The CalculiX GUI is not loaded.";
    return false;
  }

  return gui->workspace_view(view, error);
}

ReportTemplateProviderInterface* CalculiXModule::report_template_provider() noexcept
{
  return reportTemplates.get();
}

ReportModelProviderInterface* CalculiXModule::report_model_provider() noexcept
{
  return reportModels.get();
}

ProjectStateProviderInterface* CalculiXModule::project_state_provider() noexcept
{
  return this;
}
std::string CalculiXModule::project_state_id() const
{
  return "calculix";
}
void CalculiXModule::set_project_coordinated(bool coordinated)
{
  CalculiXApplicationInterface::core_observer().project_coordinated = coordinated;
}
bool CalculiXModule::restore_project_model(const ProjectStateContext& context, std::string& error)
{
  error.clear();
  if (!CalculiXApplicationInterface().core_read_cub(context.filename))
  {
    error = "CalculiX model/result restoration failed.";
    return false;
  }
  return true;
}
bool CalculiXModule::write_project_state(const ProjectStateContext& context, std::string& error)
{
  error.clear();
  if (!CalculiXApplicationInterface().core_save_cub(context.filename))
  {
    error = "CalculiX model/result persistence failed.";
    return false;
  }
  return true;
}
