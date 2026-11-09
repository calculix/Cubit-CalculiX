#ifndef CALCULIXMODULE_HPP
#define CALCULIXMODULE_HPP

#include "Framework/Interfaces/ModuleInterface.hpp"
#include "Framework/Interfaces/CommandProviderInterface.hpp"
#include "Framework/Interfaces/ComponentProviderInterface.hpp"

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"

#include <memory>

class CalculiXCommands;
class CalculiXGui;
class CalculiXResultSourceProvider;
class CalculiXJobMonitorProvider;
class CalculiXReportTemplateProvider;
class CalculiXReportModelProvider;

class CalculiXModule : public ModuleInterface,
                       public CommandProviderInterface,
                       public ComponentProviderInterface,
                       public WorkspaceProviderInterface
{
public:
  CalculiXModule();
  ~CalculiXModule() override;

  std::string module_id() const override;
  bool initialize(UserOptionsInterface& options, DrawingInterface& drawings, std::string& error) override;
  void shutdown() noexcept override;
  CommandProviderInterface* command_provider() noexcept override;
  ComponentProviderInterface* component_provider() noexcept override;
  std::vector<std::string> command_keys() const override;
  CubitCommand* create_command(const std::string& key) override;
  bool start_component(int withGUI, QWidget* host, std::string& error) override;
  void stop_component() noexcept override;
  WorkspaceProviderInterface* workspace_provider() noexcept override;
  ResultSourceProviderInterface* result_source_provider() noexcept override;
  JobMonitorProviderInterface* job_monitor_provider() noexcept override;
  ReportTemplateProviderInterface* report_template_provider() noexcept override;
  ReportModelProviderInterface* report_model_provider() noexcept override;
  bool workspace_view(WorkspaceView& view, std::string& error) override;

private:
  std::unique_ptr<CalculiXCommands> commands;
  std::unique_ptr<CalculiXGui> gui;
  std::unique_ptr<CalculiXResultSourceProvider> resultProvider;
  std::unique_ptr<CalculiXJobMonitorProvider> monitorProvider;
  std::unique_ptr<CalculiXReportTemplateProvider> reportTemplates;
  std::unique_ptr<CalculiXReportModelProvider> reportModels;
  bool observer_registered = false;
};

#endif // CALCULIXMODULE_HPP
