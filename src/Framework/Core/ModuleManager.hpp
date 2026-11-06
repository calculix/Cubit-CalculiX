#ifndef MODULEMANAGER_HPP
#define MODULEMANAGER_HPP

#include "Framework/FrameworkExport.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

class UserOptionsInterface;
class DrawingInterface;
class CommandProviderInterface;
class CubitCommand;
class ComponentProviderInterface;
class ModuleInterface;
class WorkspaceProviderInterface;
class ResultSourceProviderInterface;
class QWidget;

class CUBITSIM_FRAMEWORK_EXPORT ModuleManager : public ResultSourceRegistryInterface
{
public:
  ModuleManager();
  ~ModuleManager();

  ModuleManager(const ModuleManager&) = delete;
  ModuleManager& operator=(const ModuleManager&) = delete;

  bool add_module(std::unique_ptr<ModuleInterface> module, std::string& error);
  bool initialize(UserOptionsInterface& options, DrawingInterface& drawings, std::string& error);
  void shutdown() noexcept;

  bool start_components(int withGUI, QWidget* host, std::string& error);
  void stop_components() noexcept;
  std::vector<WorkspaceProviderInterface*> workspace_providers() const;

  // Borrowed providers, available only while initialized. Do not retain across
  // shutdown; published result handles may outlive the provider itself.
  ResultSourceProviderInterface* result_source_provider(const std::string& prefix) const noexcept;
  ResultSourceProviderInterface* application_result_source_provider(const std::string& application_id) const noexcept;
  bool resolve_result_provider(
    const std::string& prefix,
    ResolvedResultSourceProvider& output,
    std::string& error) const override;

  std::vector<std::string> command_keys() const;
  CubitCommand* create_command(const std::string& key);

private:
  std::vector<std::unique_ptr<ModuleInterface>> modules;
  std::vector<ModuleInterface*> started_modules;
  std::vector<ComponentProviderInterface*> started_components;
  std::map<std::string, CommandProviderInterface*> command_owners;
  std::map<std::string, ResultSourceProviderInterface*> result_providers;
  std::map<std::string, ResultSourceProviderInterface*> application_result_providers;

  std::vector<std::string> ordered_keys;

  bool registration_closed = false;
  bool initialized = false;
  bool components_started = false;
};

#endif // MODULEMANAGER_HPP
