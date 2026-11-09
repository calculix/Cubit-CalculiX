#pragma once
#include "Framework/Reporting/ReportModelProviderInterface.hpp"

class CalculiXReportModelProvider final : public ReportModelProviderInterface
{
public:
  CalculiXReportModelProvider();
  ~CalculiXReportModelProvider() override;
  bool available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const override;
  bool resolve_source(const ReportRequest& request, ReportSource& output, std::string& error) const override;
  bool collect(const ReportRequest& request, const ReportSource& source, ReportDocument& output, std::string& error) const override;
};
