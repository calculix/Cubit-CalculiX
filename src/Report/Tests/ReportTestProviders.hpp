#pragma once
#include "Framework/Reporting/ReportProviderRegistryInterface.hpp"
#include "Framework/Reporting/ReportTemplateProviderInterface.hpp"
#include "Framework/Reporting/ReportModelProviderInterface.hpp"

class ReportTestProviders final : public ReportProviderRegistryInterface,
  public ReportTemplateProviderInterface, public ReportModelProviderInterface
{
public:
  ReportTestProviders();
  ~ReportTestProviders() override;
  bool fail_collection = false;
  bool changed_source = false;
  bool fail_discovery = false;
  bool publish_sources = true;
  ReportProfileStoreInterface* profiles = nullptr;
  bool available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const override;
  bool available_report_sources(std::vector<ReportSourceEntry>& output, std::string& error) const override;
  bool resolve_model_reporting(const std::string& prefix, ReportTemplateProviderInterface*& templates,
    ReportModelProviderInterface*& models, std::string& error) const override;
  ReportProfileStoreInterface* report_profiles() const noexcept override;
  ReportResultProviderInterface* report_results() const noexcept override;
  ReportImageProviderInterface* report_images(const std::string& prefix) const noexcept override;
  std::string command_prefix() const override;
  bool describe(ReportTemplate& output, std::string& error) const override;
  bool resolve_source(const ReportRequest& request, ReportSource& output, std::string& error) const override;
  bool collect(const ReportRequest& request, const ReportSource& source, ReportDocument& output, std::string& error) const override;
};
