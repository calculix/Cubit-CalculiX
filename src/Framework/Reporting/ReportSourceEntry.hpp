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
  std::string unavailable_reason;
};
