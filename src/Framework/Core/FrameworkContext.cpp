#include "FrameworkContext.hpp"
#include "FrameworkModules.hpp"
#include "Framework/GUI/WorkspaceManager.hpp"

#include <exception>
#include <utility>

FrameworkContext::FrameworkContext() = default;

FrameworkContext::~FrameworkContext() = default;

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

  if (clients == 0 && !modules.initialize(error))
  {
    return false;
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
    modules.shutdown();
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
    error = "The FEA command plugin is already attached.";
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
    auto manager = std::make_unique<WorkspaceManager>();

    if (!manager->attach(host, error))
    {
      return false;
    }

    workspaces = std::move(manager);
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

  return false;
}

bool FrameworkContext::register_workspaces(std::string& error)
{
  try
  {
    if (workspaces != nullptr)
    {
      return workspaces->register_views(modules.workspace_providers(), error);
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
    error = "Workspace switching requires the FEA GUI component.";
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
  if (workspaces != nullptr)
  {
    workspaces->shutdown();
  }

  workspaces.reset();
}
