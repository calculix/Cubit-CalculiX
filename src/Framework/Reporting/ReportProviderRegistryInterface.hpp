#pragma once
#include <string>
#include <vector>
class ReportSourceEntry;
class ReportTemplate;
class ReportRequest;
class ReportSource;
class ReportDocument;
class ReportResultRequest;
class ReportImageRequest;
class ReportImage;
class ReportProfile;
class ReportTemplateProviderInterface;
class ReportModelProviderInterface;
class ReportProfileStoreInterface;
class ReportResultProviderInterface;
class ReportImageProviderInterface;

// Borrowed providers live from successful initialization until shutdown starts.
// Calls execute on the host thread. Failure leaves output unchanged; success
// clears error. Collect/render must not activate or mutate the user workspace.
class ReportProviderRegistryInterface
{
public:
  virtual ~ReportProviderRegistryInterface();
  virtual bool available_report_sources(std::vector<ReportSourceEntry>& output, std::string& error) const = 0;
  virtual bool resolve_model_reporting(const std::string& prefix, ReportTemplateProviderInterface*& templates,
    ReportModelProviderInterface*& models, std::string& error) const = 0;
  virtual ReportProfileStoreInterface* report_profiles() const noexcept = 0;
  virtual ReportResultProviderInterface* report_results() const noexcept = 0;
  virtual ReportImageProviderInterface* report_images(const std::string& prefix) const noexcept = 0;
};
