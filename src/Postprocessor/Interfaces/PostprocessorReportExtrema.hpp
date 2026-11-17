#pragma once
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
#include <atomic>
#include <string>
#include <vector>

class PostprocessorReportExtrema
{
public:
  PostprocessorReportExtrema();
  // Pure reduction: NaN/missing values are excluded. Ties use the lowest
  // original sample index, so merge order does not change the chosen location.
  void accumulate(const std::vector<double>& values, std::size_t offset,
    const std::atomic<bool>& stop);
  void merge(const PostprocessorReportExtrema& other);
  // Host-thread VTK access, at most four immutable 64K-sample buffers in flight.
  static bool collect(vtkDataSet* mesh, const SimResults::Field& raw,
    const Postprocessor::FieldSelection& selection, ReportProgress* progress,
    PostprocessorReportExtrema& output, std::string& error);
  bool available = false;
  double minimum = 0.0;
  double maximum = 0.0;
  std::size_t minimum_sample = 0;
  std::size_t maximum_sample = 0;
};
