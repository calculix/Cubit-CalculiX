#pragma once
#include "Framework/Results/VtkResultsData.hpp"
#include "Framework/Reporting/ReportConfiguration.hpp"
#include <string>
#include <vector>

class PostprocessorReportFrameSelection
{
public:
  PostprocessorReportFrameSelection();
  // CalculiX result step numbers are solver step ordinals. Canonical source
  // order, never time/frequency, determines the last retained frame of a step.
  bool select(const SimResults::VtkResultsData& source,
    const ReportConfiguration& settings, std::vector<std::size_t>& output,
    std::string& error) const;
  static bool envelope(const ReportConfiguration& settings);
};
