#include "Core/reporting/CalculiXReportImageSettings.hpp"
#include "Core/reporting/CalculiXReportImageProvider.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
#include <iostream>

int reportImageSettingsTests()
{
  int failures = 0;
  const auto check = [&](bool value, const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  std::vector<int> selected = {99};
  check(CalculiXReportImageSettings::select_steps({2, 7, 42}, "3,1,3", selected, error) && selected == std::vector<int>({2, 42}),
    "Step ordinals map to nonconsecutive IDs in model order without duplicates");
  check(!CalculiXReportImageSettings::select_steps({2, 7, 42}, "4", selected, error) && selected == std::vector<int>({2, 42}),
    "An out-of-range step ordinal preserves the previous selection");
  check(!CalculiXReportImageSettings::select_steps({2, 7, 42}, "0", selected, error), "Step ordinals are one-based");
  ReportImageRequest request;
  request.purpose = "loads";
  request.context = "step:7";
  request.settings.values = {{"ccx.images.camera", "front"}, {"ccx.images.labels", "false"}};
  CalculiXCaptureRequest capture;
  std::string id, caption;
  check(CalculiXReportImageSettings::decode_capture(request, capture, id, caption, error) &&
      capture.scope.stepId == 7 && capture.image.view == DrawingCaptureView::Front && !capture.labels.enabled &&
      id == "ccx.images.steps.7.loads", "Report capture decodes purpose, actual step ID and profile settings");
  request.context = "step:0";
  const auto previousId = id;
  check(!CalculiXReportImageSettings::decode_capture(request, capture, id, caption, error) && id == previousId && capture.scope.stepId == 7,
    "Invalid context cannot partially replace the decoded capture");
  CalculiXReportImageSettings settings;
  settings.width = 100;
  ReportConfiguration invalid;
  invalid.values = {{"ccx.images.width", "8192"}, {"ccx.images.height", "8192"}};
  check(!CalculiXReportImageSettings::decode(invalid, settings, error) && settings.width == 100,
    "The combined pixel budget is validated atomically");
  ReportDocument report;
  report.source.workspace_id = "calculix";
  report.source.kind = "job";
  report.source.job_id = 1;
  request.source = report.source;
  request.settings = {};
  CalculiXReportImageProvider provider;
  check(provider.collect(request, report, error) && report.sections.empty() && report.diagnostics.size() == 1,
    "Unverified job captures yield one diagnostic and never access current model geometry");
  check(provider.collect(request, report, error) && report.diagnostics.size() == 1,
    "Repeated image collection replaces its own diagnostics");
  const auto before = report.to_json();
  request.settings = invalid;
  check(!provider.collect(request, report, error) && report.to_json() == before,
    "Invalid image configuration preserves the complete previous document");
  request.settings.values = {{"ccx.images.model", "false"}, {"ccx.images.setup", "false"},
    {"ccx.images.loads", "false"}, {"ccx.images.bcs", "false"}};
  check(provider.collect(request, report, error) && report.diagnostics.empty(),
    "Disabling all setup images removes prior image diagnostics");
  return failures;
}
