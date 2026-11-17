#pragma once
#include <string>
class ReportDocumentLayout;
class ReportProgress;
class QIODevice;

class ReportPdfExporter
{
public:
  ReportPdfExporter();
  // encode finalizes the PDF but never closes/commits the caller-owned device.
  bool encode(const ReportDocumentLayout& layout, QIODevice& output,
    std::string& error, ReportProgress* progress = nullptr) const;
  bool write(const ReportDocumentLayout& layout, const std::string& filename,
    std::string& error, ReportProgress* progress = nullptr) const;
};
