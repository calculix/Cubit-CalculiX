#pragma once
#include "ReportSection.hpp"
#include "ReportOption.hpp"
#include "ReportConfiguration.hpp"
#include <string>
#include <vector>

class ReportTemplate
{
public:
  ReportTemplate();
  std::string id;
  std::string workspace_id;
  int revision = 1;
  std::vector<ReportSection> sections;
  std::vector<ReportOption> options;
  ReportConfiguration defaults;
};
