#pragma once
#include "Core/drawing/CalculiXCaptureRequest.hpp"
#include "Framework/Reporting/ReportImageRequest.hpp"
#include "Framework/Reporting/ReportTemplate.hpp"

class CalculiXReportImageSettings
{
public:
  CalculiXReportImageSettings();
  bool model = true;
  bool setup = true;
  bool loads = false;
  bool bcs = false;
  bool labels;
  int width;
  int height;
  std::string camera;
  bool enabled() const;
  static void add_options(ReportTemplate& definition);
  static bool decode(const ReportConfiguration& configuration, CalculiXReportImageSettings& output, std::string& error);
  static bool decode_capture(const ReportImageRequest& request, CalculiXCaptureRequest& output,
    std::string& imageId, std::string& caption, std::string& error);
  // Report selections are one-based ordinals in stored/export step order.
  // Maps to actual IDs without assuming consecutive IDs; preserves model order.
  static bool select_steps(const std::vector<int>& orderedIds, const std::string& selection,
    std::vector<int>& output, std::string& error);
};
