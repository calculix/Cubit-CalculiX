#pragma once
#include <string>
class ReportSection;
class CalculiXReportMaterials {
public:
  static bool collect(ReportSection& section, std::string& error);
};
