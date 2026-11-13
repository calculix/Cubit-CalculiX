#pragma once
#include "Framework/Reporting/ReportImageProviderInterface.hpp"

class CalculiXReportImageProvider final : public ReportImageProviderInterface
{
public:
  CalculiXReportImageProvider();
  ~CalculiXReportImageProvider() override;
  bool collect(const ReportImageRequest& request, ReportDocument& output, std::string& error) override;
  bool render(const ReportImageRequest& request, ReportImage& output, std::string& error) override;
};
