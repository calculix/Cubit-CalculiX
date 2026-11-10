#pragma once
#include "Framework/Interfaces/ModuleInterface.hpp"
#include "Framework/Interfaces/ProjectStateProviderInterface.hpp"
#include "Framework/Interfaces/CommandProviderInterface.hpp"
#include "Framework/Interfaces/ComponentProviderInterface.hpp"
#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <memory>
#include <functional>

class ReportProviderRegistryInterface;
class ReportApplicationInterface;
class ReportCommands;
class ReportGui;
class ReportModelObserver;

class ReportModule final : public ModuleInterface, public CommandProviderInterface,
  public ComponentProviderInterface, public WorkspaceProviderInterface, public ProjectStateProviderInterface
{
public:
  ProjectStateProviderInterface* project_state_provider() noexcept override;
  std::string project_state_id() const override;
  void set_project_coordinated(bool coordinated) override;
  bool prepare_project_read(const ProjectStateContext& context, std::string& error) override;
  bool commit_project_read(const ProjectStateContext& context, std::string& error) override;
  void discard_project_read() noexcept override;
  bool write_project_state(const ProjectStateContext& context, std::string& error) override;
  ReportModule(ReportProviderRegistryInterface& registry,
    std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent);
  ~ReportModule() override;
  std::string module_id() const override;
  bool initialize(UserOptionsInterface& options, DrawingInterface& drawings, std::string& error) override;
  void shutdown() noexcept override;
  CommandProviderInterface* command_provider() noexcept override;
  ComponentProviderInterface* component_provider() noexcept override;
  WorkspaceProviderInterface* workspace_provider() noexcept override;
  std::vector<std::string> command_keys() const override;
  CubitCommand* create_command(const std::string& key) override;
  bool start_component(int withGUI, QWidget* host, std::string& error) override;
  void stop_component() noexcept override;
  bool workspace_view(WorkspaceView& view, std::string& error) override;

private:
  ReportProviderRegistryInterface& registry;
  std::string componentDirectory;
  std::function<bool(const std::string&, std::string&)> submit;
  std::function<bool(const std::string&, std::string&)> silent;
  std::shared_ptr<ReportApplicationInterface> application;
  std::unique_ptr<ReportCommands> commands;
  std::unique_ptr<ReportGui> gui;
  std::unique_ptr<ReportModelObserver> modelObserver;
};
