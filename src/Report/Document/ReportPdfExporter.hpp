#pragma once
#include <string>
class ReportDocumentLayout;

class ReportPdfExporter
{
public:
  ReportPdfExporter();
  bool write(const ReportDocumentLayout& layout, const std::string& filename, std::string& error) const;
};
