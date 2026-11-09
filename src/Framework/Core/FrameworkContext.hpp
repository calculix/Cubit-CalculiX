#ifndef FRAMEWORKCONTEXT_HPP
#define FRAMEWORKCONTEXT_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include "Framework/FrameworkExport.hpp"
#include "ModuleManager.hpp"
#include "Monitoring/FrameworkUserOptions.hpp"
#include <QPointer>

#include <memory>
#include <cstddef>
#include <string>

class UserOptionsManager;
class DrawingService;
class UserOptionsStore;
class ReportProfileStore;
class WorkspaceManager;
class QWidget;
class ResourceStatusWidget;

class CUBITSIM_FRAMEWORK_EXPORT FrameworkContext
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
  FrameworkOptions user_options() const noexcept;
  std::vector<UserOption> user_option_definitions() const;
  bool user_option_values(UserOptionValues& snapshot, std::string& error) const;
  bool apply_user_options(const UserOptionValues& changes, std::string& error);

private:
  // Declared before modules so the service outlives module destruction.
  std::unique_ptr<UserOptionsStore> options;
  std::unique_ptr<ReportProfileStore> reportProfiles;
  std::unique_ptr<FrameworkUserOptions> preferences;
  std::unique_ptr<DrawingService> drawings;
  ModuleManager modules;
  std::unique_ptr<WorkspaceManager> workspaces;
  std::unique_ptr<UserOptionsManager> optionsPages;
  QPointer<ResourceStatusWidget> resourceStatus;
  std::size_t clients = 0;
  bool registered = false;
  bool command_plugin_attached = false;
};

// Implemented in the shared framework library, never in either entry-point DLL.
CUBITSIM_FRAMEWORK_EXPORT FrameworkContext& framework_context();

#endif // FRAMEWORKCONTEXT_HPP
