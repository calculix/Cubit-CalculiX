#pragma once
#include <string>
class ReportSection;
class ReportProgress;
class CalculiXReportMaterials {
public:
  static bool collect(ReportSection& section, std::string& error, ReportProgress* progress = nullptr);
};
