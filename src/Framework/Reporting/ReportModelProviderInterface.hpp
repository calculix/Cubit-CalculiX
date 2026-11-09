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
class ReportProfileStoreInterface;
class ReportResultProviderInterface;
class ReportImageProviderInterface;

// Borrowed providers live from successful initialization until shutdown starts.
// Calls execute on the host thread. Failure leaves output unchanged; success
// clears error. Collect/render must not activate or mutate the user workspace.
class ReportModelProviderInterface
{
public:
  virtual ~ReportModelProviderInterface();
  virtual bool available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const = 0;
  virtual bool resolve_source(const ReportRequest& request, ReportSource& output, std::string& error) const = 0;
  virtual bool collect(const ReportRequest& request, const ReportSource& source, ReportDocument& output, std::string& error) const = 0;
};
