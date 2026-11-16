#pragma once
#include <string>
class ReportConfiguration;
class ReportDocument;

class ReportUnitValidation
{
public:
  ReportUnitValidation();
  static bool configuration(const ReportConfiguration& value, std::string& error);
  // Validates supplied physical assertions without inferring solver quantities.
  static bool document(const ReportDocument& value, std::string& error);
};
