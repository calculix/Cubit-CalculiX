#ifndef MODULEMANAGER_HPP
#define MODULEMANAGER_HPP

#include "Framework/FrameworkExport.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"

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
class JobMonitorProviderInterface;
class QWidget;

class CUBITSIM_FRAMEWORK_EXPORT ModuleManager : public ResultSourceRegistryInterface, public ReportProviderRegistryInterface
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
  std::vector<JobMonitorProviderInterface*> job_monitor_providers() const;

  // Borrowed providers, available only while initialized. Do not retain across
  // shutdown; published result handles may outlive the provider itself.
  ResultSourceProviderInterface* result_source_provider(const std::string& prefix) const noexcept;
  ResultSourceProviderInterface* application_result_source_provider(const std::string& application_id) const noexcept;
  bool resolve_result_provider(
    const std::string& prefix,
    ResolvedResultSourceProvider& output,
    std::string& error) const override;

  bool available_report_sources(std::vector<ReportSourceEntry>& output, std::string& error) const override;
  void bind_report_profiles(ReportProfileStoreInterface* store) noexcept;
  bool resolve_model_reporting(const std::string& prefix, ReportTemplateProviderInterface*& templates,
    ReportModelProviderInterface*& models, std::string& error) const override;
  ReportProfileStoreInterface* report_profiles() const noexcept override;
  ReportResultProviderInterface* report_results() const noexcept override;
  ReportImageProviderInterface* report_images(const std::string& prefix) const noexcept override;

  std::vector<std::string> command_keys() const;
  CubitCommand* create_command(const std::string& key);

private:
  std::vector<std::unique_ptr<ModuleInterface>> modules;
  std::vector<ModuleInterface*> started_modules;
  std::vector<ComponentProviderInterface*> started_components;
  std::map<std::string, CommandProviderInterface*> command_owners;
  std::map<std::string, ResultSourceProviderInterface*> result_providers;
  std::map<std::string, ResultSourceProviderInterface*> application_result_providers;

  std::map<std::string, ReportTemplateProviderInterface*> report_templates;
  std::map<std::string, ReportModelProviderInterface*> report_models;
  std::map<std::string, ReportImageProviderInterface*> report_image_providers;
  ReportResultProviderInterface* result_report_provider = nullptr;
  ReportProfileStoreInterface* profile_store = nullptr;
  std::vector<std::string> ordered_keys;

  bool registration_closed = false;
  bool initialized = false;
  bool components_started = false;
};

#endif // MODULEMANAGER_HPP
