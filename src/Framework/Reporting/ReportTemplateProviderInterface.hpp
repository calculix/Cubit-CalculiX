#pragma once
#include <string>
#include <vector>
class ReportTemplate;
class ReportRequest;
class ReportSource;
class ReportDocument;
class ReportResultRequest;
class ReportImageRequest;
class ReportImage;
class ReportProfile;
class ReportModelProviderInterface;
class ReportProfileStoreInterface;
class ReportResultProviderInterface;
class ReportImageProviderInterface;

// Borrowed providers live from successful initialization until shutdown starts.
// Calls execute on the host thread. Failure leaves output unchanged; success
// clears error. Collect/render must not activate or mutate the user workspace.
class ReportTemplateProviderInterface
{
public:
  virtual ~ReportTemplateProviderInterface();
  virtual std::string command_prefix() const = 0;
  virtual bool describe(ReportTemplate& output, std::string& error) const = 0;
};
