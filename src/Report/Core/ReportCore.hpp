#pragma once
#include "ReportSession.hpp"
#include "Framework/Reporting/ReportSourceEntry.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"
#include "Framework/Reporting/ReportProfile.hpp"

class ReportProviderRegistryInterface;

class ReportCore
{
public:
  explicit ReportCore(ReportProviderRegistryInterface& registry);
  ~ReportCore();
  const ReportSession& session() const noexcept;
  std::shared_ptr<const ReportDocument> document() const noexcept;
  bool describe(ReportTemplate& output, std::string& error) const;
  const ReportRequest& configuration_request() const noexcept;
  bool available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const;
  bool choose_source(const ReportRequest& source, std::string& error);
  bool generate_request(const ReportRequest& source, const std::string& profile, std::string& error);
  bool validate_configuration_patch(const ReportConfiguration& patch, std::string& error) const;
  bool configure_patch(const ReportConfiguration& patch, std::string& error);
  bool generate(const std::string& provider, int job, const std::string& profile, std::string& error);
  bool update(std::string& error);
  bool configure(const std::string& key, const std::string& value, std::string& error);
  bool profile(const std::string& action, const std::string& name, std::vector<ReportProfile>& output, std::string& error);
  bool note(const std::string& section, const std::string& text, std::string& error);
  void restore(ReportSession session);
  void reset();

private:
  std::shared_ptr<const ReportDocument> find_document(const ReportRequest& source, const std::string& workspace = "") const;
  bool activate_document(const std::string& id, std::string& error);
  bool build(const ReportRequest& request, const ReportProfile* profile, bool updating, std::string& error);
  ReportProviderRegistryInterface& registry;
  ReportSession state;
};
