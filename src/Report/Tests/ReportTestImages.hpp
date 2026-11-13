#pragma once
#include "Framework/Reporting/ReportImageProviderInterface.hpp"

class ReportTestImages final : public ReportImageProviderInterface
{
public:
  ReportTestImages();
  ~ReportTestImages() override;
  bool fail = false;
  bool collect(const ReportImageRequest& request, ReportDocument& output, std::string& error) override;
  bool render(const ReportImageRequest& request, ReportImage& output, std::string& error) override;
};
