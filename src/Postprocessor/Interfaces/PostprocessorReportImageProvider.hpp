#pragma once
#include "Framework/Reporting/ReportImageProviderInterface.hpp"
#include <memory>
class PostprocessorApplicationInterface;
class PostprocessorReportImageProvider final : public ReportImageProviderInterface
{
public:
  explicit PostprocessorReportImageProvider(std::weak_ptr<PostprocessorApplicationInterface> application);
  bool collect(const ReportImageRequest&,ReportDocument&,std::string& error) override;
  bool render(const ReportImageRequest& request,ReportImage& output,std::string& error) override;
private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};
