#pragma once
#include <string>
class ReportRequest;
class ReportSource;
class ReportDocument;

class CalculiXReportModelSnapshot
{
public:
  CalculiXReportModelSnapshot();
  // Captures read-only Core values once on the host thread. Does not prepare
  // export, write solver input, create a temporary job, or evaluate results.
  bool collect(const ReportRequest& request, const ReportSource& source, ReportDocument& output, std::string& error) const;
};
