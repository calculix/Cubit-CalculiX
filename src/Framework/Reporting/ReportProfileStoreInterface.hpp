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
class ReportResultProviderInterface;
class ReportImageProviderInterface;

// Borrowed providers live from successful initialization until shutdown starts.
// Calls execute on the host thread. Failure leaves output unchanged; success
// clears error. Collect/render must not activate or mutate the user workspace.
class ReportProfileStoreInterface
{
public:
  virtual ~ReportProfileStoreInterface();
  virtual bool list(const std::string& workspace, std::vector<ReportProfile>& output, std::string& error) const = 0;
  virtual bool save(ReportProfile& profile, std::string& error) = 0;
  virtual bool load(const std::string& workspace, const std::string& name, ReportProfile& output, std::string& error) const = 0;
  virtual bool default_profile(const std::string& workspace, ReportProfile& output, bool& found, std::string& error) const = 0;
  virtual bool set_default_profile(const std::string& workspace, const std::string& name, std::string& error) = 0;
};
