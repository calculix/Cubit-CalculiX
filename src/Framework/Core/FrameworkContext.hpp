#ifndef FRAMEWORKCONTEXT_HPP
#define FRAMEWORKCONTEXT_HPP

#include "Framework/FrameworkExport.hpp"
#include "ModuleManager.hpp"

#include <memory>
#include <cstddef>
#include <string>

class UserOptionsManager;
class UserOptionsStore;
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
  // Success means the desktop accepted the request, not that rendering completed.
  bool open_documentation(std::string& error) const;

private:
  // Declared before modules so the service outlives module destruction.
  std::unique_ptr<UserOptionsStore> options;
  ModuleManager modules;
  std::unique_ptr<WorkspaceManager> workspaces;
  std::unique_ptr<UserOptionsManager> optionsPages;
  std::size_t clients = 0;
  bool registered = false;
  bool command_plugin_attached = false;
};

// Implemented in the shared framework library, never in either entry-point DLL.
FEA_FRAMEWORK_EXPORT FrameworkContext& framework_context();

#endif // FRAMEWORKCONTEXT_HPP
