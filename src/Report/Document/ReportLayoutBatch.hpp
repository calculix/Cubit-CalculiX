#pragma once
#include "Framework/Reporting/ReportDocument.hpp"
#include <memory>
#include <vector>

class ReportDocumentLayout;
class ReportProgress;

class ReportLayoutBatch
{
public:
  ReportLayoutBatch();
  ~ReportLayoutBatch();
  static bool plan(const ReportDocument& document,
    std::vector<std::unique_ptr<ReportLayoutBatch>>& output,
    ReportProgress* progress, std::string& error);
  bool prepare(std::string& error);
  bool resolve_contents(const std::vector<std::pair<std::string, int>>& pages, std::string& error);
  const ReportDocumentLayout& layout() const;
  int first_page = 0;

private:
  ReportDocument fragment;
  bool contents_only = false;
  std::unique_ptr<ReportDocumentLayout> prepared;
};
