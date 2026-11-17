#pragma once
#include "Framework/Reporting/ReportResultRequest.hpp"
#include "Framework/Reporting/ReportSection.hpp"
#include "Framework/Reporting/ReportDiagnostic.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include <array>
#include <memory>

class PostprocessorApplicationInterface;
class PostprocessorReportBlockSummary
{
public:
  PostprocessorReportBlockSummary();
  static bool source(const PostprocessorApplicationInterface& application,
    const ReportSource& requested, std::shared_ptr<const SimResults::VtkResultsData>& output,
    std::string& error);
  // All output is private until the entire collection succeeds. Raw source
  // arrays stay immutable; cross-frame comparisons use Report display units.
  static bool collect(const ReportResultRequest& request,
    const SimResults::VtkResultsData& source,
    std::vector<PostprocessorReportBlockSummary>& output, std::string& error,
    std::vector<ReportDiagnostic>* diagnostics = nullptr);
  ReportSection section(bool values) const;
  std::string id;
  SimResults::Dataset block;
  SimResults::Field field;
  std::string component;
  std::string unit;
  std::string coordinate_unit;
  bool available = false;
  double minimum = 0.0;
  double maximum = 0.0;
  std::size_t minimum_frame = 0;
  std::size_t maximum_frame = 0;
  std::string minimum_id;
  std::string maximum_id;
  std::array<QJsonValue, 3> minimum_position;
  std::array<QJsonValue, 3> maximum_position;
};
