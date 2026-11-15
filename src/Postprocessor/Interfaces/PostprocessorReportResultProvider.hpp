#pragma once
#include "Framework/Reporting/ReportResultProviderInterface.hpp"
#include <memory>
class PostprocessorApplicationInterface;
class PostprocessorReportResultProvider : public ReportResultProviderInterface {
public:
  explicit PostprocessorReportResultProvider(std::weak_ptr<PostprocessorApplicationInterface> application);
  bool collect(const ReportResultRequest& request, ReportDocument& output, std::string& error) const override;
private:
  std::weak_ptr<PostprocessorApplicationInterface> application;
};
