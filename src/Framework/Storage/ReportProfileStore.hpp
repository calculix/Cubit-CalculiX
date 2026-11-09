#pragma once
#include "StorageExport.hpp"
#include "Framework/Reporting/ReportProfileStoreInterface.hpp"
#include <string>

class CUBITSIM_STORAGE_EXPORT ReportProfileStore final : public ReportProfileStoreInterface
{
public:
  explicit ReportProfileStore(std::string filename);
  ~ReportProfileStore() override;
  bool list(const std::string& workspace, std::vector<ReportProfile>& output, std::string& error) const override;
  bool save(ReportProfile& profile, std::string& error) override;
  bool load(const std::string& workspace, const std::string& name, ReportProfile& output, std::string& error) const override;
  bool default_profile(const std::string& workspace, ReportProfile& output, bool& found, std::string& error) const override;
  bool set_default_profile(const std::string& workspace, const std::string& name, std::string& error) override;

private:
  std::string filename;
};
