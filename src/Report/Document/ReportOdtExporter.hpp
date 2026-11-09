#pragma once
#include <string>
class ReportDocumentLayout;

class ReportOdtExporter
{
public:
  ReportOdtExporter();
  // Native ODF headings, numeric tables, embedded images, master-page header
  // and footer. Pagination remains the responsibility of the ODT editor.
  bool write(const ReportDocumentLayout& layout, const std::string& filename, std::string& error) const;
};
