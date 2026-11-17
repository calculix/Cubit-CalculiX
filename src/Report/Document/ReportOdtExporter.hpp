#pragma once
#include <string>
class ReportDocument;
class ReportProgress;
class QIODevice;

class ReportOdtExporter
{
public:
  ReportOdtExporter();
  // Semantic document export: no preview/pagination or value reconversion.
  // encode owns no output device and never commits the caller's temporary file.
  bool encode(const ReportDocument& report, QIODevice& output,
    std::string& error, ReportProgress* progress = nullptr) const;
  bool write(const ReportDocument& report, const std::string& filename,
    std::string& error, ReportProgress* progress = nullptr) const;
};
