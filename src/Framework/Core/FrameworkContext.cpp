#include "FrameworkContext.hpp"
#include "FrameworkModules.hpp"
#include "Framework/Drawing/DrawingService.hpp"
#include "Framework/Storage/UserOptionsStore.hpp"
#include "Framework/Utility/FrameworkPaths.hpp"
#include "Framework/GUI/UserOptionsManager.hpp"
#include "Framework/GUI/WorkspaceManager.hpp"
#include "Framework/GUI/Status/ResourceStatusWidget.hpp"
#include "CubitMessage.hpp"
#include <QMainWindow>
#include <QStatusBar>

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QUrl>
#include <exception>
#include <utility>

namespace
{
int componentAnchor = 0;
}

FrameworkContext::FrameworkContext() = default;

FrameworkContext::~FrameworkContext()
{
  detach_workspace_host();
  modules.shutdown();
}

bool FrameworkContext::acquire(std::string& error)
{
  error.clear();

  try
  {
    if (!registered)
    {
      if (!register_framework_modules(modules, error))
      {
        return false;
      }

      registered = true;
    }
  }
  catch (const std::exception& exception)
  {
    error = std::string("Module registration failed: ") + exception.what();
    return false;
  }
  catch (...)
  {
    error = "Module registration failed with an unknown exception.";
    return false;
  }

  if (clients == 0)
  {
    FrameworkPaths paths;
    if (!FrameworkPaths::resolve(&componentAnchor, paths, error))
    {
      return false;
    }
    options = std::make_unique<UserOptionsStore>(std::move(paths));
    preferences = std::make_unique<FrameworkUserOptions>();
    if (!preferences->initialize(*options, error))
    {
      preferences.reset();
      options.reset();
      return false;
    }
    drawings = std::make_unique<DrawingService>();
    if (!modules.initialize(*options, *drawings, error))
    {
      drawings.reset();
      preferences.reset();
      options.reset();
      return false;
    }
  }

  ++clients;
  return true;
}

void FrameworkContext::release() noexcept
{
  if (clients == 0)
  {
    return;
  }

  --clients;

  if (clients == 0)
  {
    detach_workspace_host();
    modules.shutdown();
    drawings.reset();
    preferences.reset();
    options.reset();
  }
}

ModuleManager& FrameworkContext::module_manager() noexcept
{
  return modules;
}

FrameworkContext& framework_context()
{
  static FrameworkContext context;
  return context;
}

bool FrameworkContext::acquire_command_plugin(std::string& error)
{
  if (command_plugin_attached)
  {
    error = "The cubit-sim command plugin is already attached.";
    return false;
  }

  if (!acquire(error))
  {
    return false;
  }

  command_plugin_attached = true;
  return true;
}

void FrameworkContext::release_command_plugin() noexcept
{
  if (command_plugin_attached)
  {
    command_plugin_attached = false;
    release();
  }
}

bool FrameworkContext::has_command_plugin() const noexcept
{
  return command_plugin_attached;
}

bool FrameworkContext::attach_workspace_host(QWidget* host, std::string& error)
{
  error.clear();

  if (workspaces != nullptr)
  {
    error = "The framework already has a workspace host.";
    return false;
  }

  try
  {
    auto manager = std::make_unique<WorkspaceManager>(*drawings);

    if (!manager->attach(host, error))
    {
      return false;
    }

    workspaces = std::move(manager);
    auto* window = qobject_cast<QMainWindow*>(host);
    if (window == nullptr || preferences == nullptr)
    {
      error = "Resource status requires the Framework preferences and main window.";
      detach_workspace_host();
      return false;
    }
    resourceStatus = new ResourceStatusWidget(window->statusBar(), modules, *preferences);
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot attach the workspace host: ") + exception.what();
  }
  catch (...)
  {
    error = "Cannot attach the workspace host.";
  }

  detach_workspace_host();
  return false;
}

bool FrameworkContext::register_workspaces(std::string& error)
{
  try
  {
    if (workspaces != nullptr)
    {
      const auto providers = modules.workspace_providers();
      if (!workspaces->register_views(providers, error))
      {
        return false;
      }
      optionsPages = std::make_unique<UserOptionsManager>();
      const auto frameworkPages = resourceStatus != nullptr
        ? std::vector<WorkspaceView>{resourceStatus->options_page()} : std::vector<WorkspaceView>{};
      return optionsPages->attach(providers, error, frameworkPages);
    }

    error = "No GUI workspace host is attached.";
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot register workspaces: ") + exception.what();
  }
  catch (...)
  {
    error = "Cannot register workspaces.";
  }

  return false;
}

bool FrameworkContext::select_workspace(const std::string& id, std::string& error)
{
  if (workspaces == nullptr)
  {
    error = "Workspace switching requires the cubit-sim GUI component.";
    return false;
  }

  try
  {
    return workspaces->activate(QString::fromStdString(id), error);
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot select the workspace: ") + exception.what();
  }
  catch (...)
  {
    error = "Cannot select the workspace.";
  }

  return false;
}

void FrameworkContext::detach_workspace_host() noexcept
{
  optionsPages.reset();
  delete resourceStatus.data();
  resourceStatus = nullptr;
  if (workspaces != nullptr)
  {
    workspaces->shutdown();
  }

  workspaces.reset();
}

FrameworkOptions FrameworkContext::user_options() const noexcept
{
  return preferences != nullptr ? preferences->current() : FrameworkOptions{};
}

bool FrameworkContext::apply_user_options(const UserOptionValues& changes, std::string& error)
{
  if (preferences == nullptr)
  {
    error = "Framework preferences are unavailable.";
    return false;
  }
  if (!preferences->apply(changes, error))
  {
    return false;
  }
  // Saving is committed. A display-refresh failure must not report a failed save.
  try
  {
    if (resourceStatus != nullptr)
    {
      resourceStatus->apply_options();
    }
  }
  catch (const std::exception& exception)
  {
    PRINT_WARNING("Framework preferences saved, but monitoring could not restart: %s\n", exception.what());
  }
  return true;
}

bool FrameworkContext::open_documentation(std::string& error) const
{
  error.clear();
  if (options == nullptr)
  {
    error = "The Framework component directory is unavailable.";
    return false;
  }
  if (qobject_cast<QGuiApplication*>(QCoreApplication::instance()) == nullptr)
  {
    error = "Opening documentation requires a GUI-capable session.";
    return false;
  }
  const QDir component(QString::fromUtf8(options->component_directory().c_str()));
  const QFileInfo index(component.filePath("documentation/index.html"));
  if (!index.isFile() || !index.isReadable())
  {
    error = "Installed documentation is missing or unreadable: " + index.absoluteFilePath().toUtf8().toStdString();
    return false;
  }
  if (!QDesktopServices::openUrl(QUrl::fromLocalFile(index.absoluteFilePath())))
  {
    error = "The desktop could not open: " + index.absoluteFilePath().toUtf8().toStdString();
    return false;
  }
  return true;
}

std::vector<UserOption> FrameworkContext::user_option_definitions() const
{
  return FrameworkUserOptions::definitions();
}

bool FrameworkContext::user_option_values(UserOptionValues& snapshot, std::string& error) const
{
  if (preferences == nullptr)
  {
    error = "Framework preferences are unavailable.";
    return false;
  }
  return preferences->values(snapshot, error);
}
