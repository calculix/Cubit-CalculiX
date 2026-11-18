#pragma once
#include "Postprocessor/Core/capture/CaptureDefinition.hpp"
#include "Framework/Reporting/ReportConfiguration.hpp"

// One immutable presentation per collection. Samples share immutable result
// buffers, but own their presentation and subsequently evaluated geometry.
class PostprocessorReportCapturePlan
{
public:
  bool initialize(Postprocessor::CaptureSnapshot snapshot, const std::string& source,
    const ReportConfiguration& settings, std::string& error);
  // Scalar extrema always belong to the original block. Filters, deformation
  // and vectors affect only its image; they do not redefine numerical scope.
  // Failure leaves output (and the previously initialized plan) unchanged.
  bool prepare(std::size_t frame, std::size_t dataset,
    const Postprocessor::FieldSelection& field, Postprocessor::CaptureSnapshot& output,
    std::string& error) const;

private:
  Postprocessor::CaptureSnapshot base;
  std::string sourceId;
  std::string representationId;
  bool inherited = false;
};
