#pragma once
#include "Framework/Reporting/ReportConfiguration.hpp"
#include <QString>
#include <string>

// Readable transport only; Report Core owns defaults, schemas and validation.
// The caller validates a patch through ReportApplicationInterface first.
class ReportSettingsCommandText
{
public:
  static bool build(const ReportConfiguration& patch, const QString& logoFile,
    std::string& output, std::string& error);
};
