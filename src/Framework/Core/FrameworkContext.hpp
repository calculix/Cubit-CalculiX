#ifndef FRAMEWORKCONTEXT_HPP
#define FRAMEWORKCONTEXT_HPP

#include "Framework/FrameworkExport.hpp"
#include "ModuleManager.hpp"

#include <memory>
#include <cstddef>
#include <string>

class WorkspaceManager;
class QWidget;

class FEA_FRAMEWORK_EXPORT FrameworkContext
{
public:
  FrameworkContext();
  ~FrameworkContext();

  FrameworkContext(const FrameworkContext&) = delete;
  FrameworkContext& operator=(const FrameworkContext&) = delete;

  bool acquire(std::string& error);
  void release() noexcept;
  bool acquire_command_plugin(std::string& error);
  void release_command_plugin() noexcept;
  bool has_command_plugin() const noexcept;
  ModuleManager& module_manager() noexcept;
  bool attach_workspace_host(QWidget* host, std::string& error);
  bool register_workspaces(std::string& error);
  bool select_workspace(const std::string& id, std::string& error);
  void detach_workspace_host() noexcept;

private:
  ModuleManager modules;
  std::unique_ptr<WorkspaceManager> workspaces;
  std::size_t clients = 0;
  bool registered = false;
  bool command_plugin_attached = false;
};

// Implemented in the shared framework library, never in either entry-point DLL.
FEA_FRAMEWORK_EXPORT FrameworkContext& framework_context();

#endif // FRAMEWORKCONTEXT_HPP
