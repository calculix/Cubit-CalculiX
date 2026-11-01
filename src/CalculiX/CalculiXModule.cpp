#include "CalculiXModule.hpp"
#include "Commands/CalculiXCommands.hpp"
#include "GUI/CalculiXGui.hpp"
#include "CalculiXSettings.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/application/CoreObserver.hpp"
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

bool CalculiXModule::initialize(std::string& error)
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

      if (std::find(keys.begin(), keys.end(), "ccxVersionCommand") != keys.end())
      {
        error = "CalculiX commands are already registered. Restart Cubit using only the FEA plugin directory.";
        return false;
      }
    }
  }

  CalculiXSettings::restore();
  CalculiXApplicationInterface application;

  if (!application.initialize_application())
  {
    error = "Cannot initialize the CalculiX core and Cubit export/material interfaces.";
    return false;
  }

  commands = std::make_unique<CalculiXCommands>();
  application.core_observer().register_observer();
  observer_registered = true;
  return true;
}

void CalculiXModule::shutdown() noexcept
{
  stop_component();

  if (observer_registered)
  {
    CalculiXApplicationInterface application;
    application.core_observer().unregister_observer();
    observer_registered = false;
  }

  commands.reset();
  // Project data remains owned by the one ccx_core in calculix_core.
  // GUI detach does not reset the model or terminate running solver jobs.
}

CommandProviderInterface* CalculiXModule::command_provider() noexcept
{
  return this;
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
