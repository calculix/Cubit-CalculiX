#ifndef MODULEMANAGER_HPP
#define MODULEMANAGER_HPP

#include "Framework/FrameworkExport.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

class CommandProviderInterface;
class CubitCommand;
class ComponentProviderInterface;
class ModuleInterface;
class WorkspaceProviderInterface;
class QWidget;

class FEA_FRAMEWORK_EXPORT ModuleManager
{
public:
  ModuleManager();
  ~ModuleManager();

  ModuleManager(const ModuleManager&) = delete;
  ModuleManager& operator=(const ModuleManager&) = delete;

  bool add_module(std::unique_ptr<ModuleInterface> module, std::string& error);
  bool initialize(std::string& error);
  void shutdown() noexcept;

  bool start_components(int withGUI, QWidget* host, std::string& error);
  void stop_components() noexcept;
  std::vector<WorkspaceProviderInterface*> workspace_providers() const;

  std::vector<std::string> command_keys() const;
  CubitCommand* create_command(const std::string& key);

private:
  std::vector<std::unique_ptr<ModuleInterface>> modules;
  std::vector<ModuleInterface*> started_modules;
  std::vector<ComponentProviderInterface*> started_components;
  std::map<std::string, CommandProviderInterface*> command_owners;

  std::vector<std::string> ordered_keys;

  bool registration_closed = false;
  bool initialized = false;
  bool components_started = false;
};

#endif // MODULEMANAGER_HPP
