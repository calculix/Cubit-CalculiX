#ifndef POSTPROCESSORMODULE_HPP
#define POSTPROCESSORMODULE_HPP

#include "Framework/Interfaces/ModuleInterface.hpp"
#include "Framework/Interfaces/ProjectStateProviderInterface.hpp"
#include "Framework/Interfaces/CommandProviderInterface.hpp"
#include "Framework/Interfaces/ComponentProviderInterface.hpp"
#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"

#include <memory>
#include <functional>

class ResultSourceRegistryInterface;
class PostprocessorApplicationInterface;
class PostprocessorReportImageProvider;
class PostprocessorCommands;
class PostprocessorGui;
class PostprocessorModelObserver;

class PostprocessorModule : public ModuleInterface, public CommandProviderInterface,
  public ComponentProviderInterface, public WorkspaceProviderInterface, public ProjectStateProviderInterface
{
public:
  PostprocessorModule(ResultSourceRegistryInterface& providers,
    std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent);
  ~PostprocessorModule() override;

  std::string module_id() const override;
  ReportImageProviderInterface* report_image_provider() noexcept override;
  ProjectStateProviderInterface* project_state_provider() noexcept override;
  std::string project_state_id() const override;
  bool prepare_project_read(const ProjectStateContext& context, std::string& error) override;
  bool resolve_project_sources(const ProjectStateContext& context, std::string& error) override;
  bool commit_project_read(const ProjectStateContext& context, std::string& error) override;
  void discard_project_read() noexcept override;
  bool prepare_project_save(const ProjectStateContext& context, std::string& error) override;
  bool write_project_state(const ProjectStateContext& context, std::string& error) override;
  bool initialize(UserOptionsInterface& options, DrawingInterface& drawings, std::string& error) override;
  void shutdown() noexcept override;
  CommandProviderInterface* command_provider() noexcept override;
  ComponentProviderInterface* component_provider() noexcept override;
  std::vector<std::string> command_keys() const override;
  CubitCommand* create_command(const std::string& key) override;
  bool start_component(int withGUI, QWidget* host, std::string& error) override;
  void stop_component() noexcept override;
  WorkspaceProviderInterface* workspace_provider() noexcept override;
  bool workspace_view(WorkspaceView& view, std::string& error) override;

private:
  ResultSourceRegistryInterface& providers;
  std::shared_ptr<PostprocessorApplicationInterface> application;
  std::unique_ptr<PostprocessorCommands> commands;
  std::unique_ptr<PostprocessorReportImageProvider> imageProvider;
  std::unique_ptr<PostprocessorGui> gui;
  std::unique_ptr<PostprocessorModelObserver> modelObserver;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  std::function<bool(const std::string&, std::string&)> silentCommand;
};

#endif // POSTPROCESSORMODULE_HPP
