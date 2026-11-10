#pragma once
#include "ReportRequest.hpp"
#include <string>

class ReportSourceEntry
{
public:
  ReportSourceEntry();
  ReportRequest request;
  std::string workspace_label;
  std::string label;
  bool available = false;
  // A cached report remains selectable even when regeneration is unavailable.
  bool has_report = false;
  std::string unavailable_reason;
};
