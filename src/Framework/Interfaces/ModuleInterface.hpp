#ifndef MODULEINTERFACE_HPP
#define MODULEINTERFACE_HPP

#include <string>

class UserOptionsInterface;
class DrawingInterface;
class CommandProviderInterface;
class ComponentProviderInterface;
class WorkspaceProviderInterface;
class ResultSourceProviderInterface;
class JobMonitorProviderInterface;
class ReportTemplateProviderInterface;
class ReportModelProviderInterface;
class ReportResultProviderInterface;
class ReportImageProviderInterface;

class ModuleInterface
{
public:
  virtual ~ModuleInterface() = default;

  // Stable identifier, independent of translated display text.
  virtual std::string module_id() const = 0;

  virtual bool initialize(UserOptionsInterface& options, DrawingInterface& drawings, std::string& error) = 0;

  // Also valid after partial initialization; implementations must not throw.
  virtual void shutdown() noexcept = 0;

  // Providers are borrowed and remain valid until this module is destroyed.
  virtual CommandProviderInterface* command_provider() noexcept = 0;
  virtual ComponentProviderInterface* component_provider() noexcept = 0;

  virtual WorkspaceProviderInterface* workspace_provider() noexcept
  {
    return nullptr;
  }

  // Borrowed only while initialized. Queries run on the application owner thread.
  virtual JobMonitorProviderInterface* job_monitor_provider() noexcept
  {
    return nullptr;
  }

  // Reporting providers are optional and independent of loaded GUI components.
  virtual ReportTemplateProviderInterface* report_template_provider() noexcept
  {
    return nullptr;
  }
  virtual ReportModelProviderInterface* report_model_provider() noexcept
  {
    return nullptr;
  }
  virtual ReportResultProviderInterface* report_result_provider() noexcept
  {
    return nullptr;
  }
  virtual ReportImageProviderInterface* report_image_provider() noexcept
  {
    return nullptr;
  }

  // Borrowed after successful initialization, until shutdown begins.
  // Result loading does not require this module's GUI component to be started.
  virtual ResultSourceProviderInterface* result_source_provider() noexcept
  {
    return nullptr;
  }
};

#endif // MODULEINTERFACE_HPP
