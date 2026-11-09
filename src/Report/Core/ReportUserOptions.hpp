#pragma once
#include "Framework/Reporting/ReportOption.hpp"
#include <vector>

class ReportUserOptions
{
public:
  ReportUserOptions();
  static std::vector<ReportOption> definitions();
  static int initial_preview_page();
  static int initial_preview_zoom();
  static std::string initial_preview_mode();
  static int preview_margin();
  static int preview_gap();
};
