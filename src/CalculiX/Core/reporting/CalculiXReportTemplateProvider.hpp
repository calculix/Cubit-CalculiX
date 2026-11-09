#pragma once
#include "Framework/Reporting/ReportTemplateProviderInterface.hpp"

class CalculiXReportTemplateProvider final : public ReportTemplateProviderInterface
{
public:
  CalculiXReportTemplateProvider();
  ~CalculiXReportTemplateProvider() override;
  std::string command_prefix() const override;
  bool describe(ReportTemplate& output, std::string& error) const override;
};
