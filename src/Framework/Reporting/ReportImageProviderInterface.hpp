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
class ReportTemplateProviderInterface;
class ReportModelProviderInterface;
class ReportProfileStoreInterface;
class ReportResultProviderInterface;

// Borrowed providers live from successful initialization until shutdown starts.
// Calls execute on the host thread. Failure leaves output unchanged; success
// clears error. Collect/render must not activate or mutate the user workspace.
class ReportImageProviderInterface
{
public:
  virtual ~ReportImageProviderInterface();
  // Collect workspace-owned images into a local document candidate. Invalid
  // settings fail atomically; unavailable captures may add diagnostics.
  virtual bool collect(const ReportImageRequest& request, ReportDocument& output, std::string& error) = 0;
  virtual bool render(const ReportImageRequest& request, ReportImage& output, std::string& error) = 0;
};
