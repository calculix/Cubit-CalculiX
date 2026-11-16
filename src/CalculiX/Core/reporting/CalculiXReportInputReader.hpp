#pragma once
#include "Framework/Reporting/ReportSource.hpp"
#include <QJsonArray>
#include "Framework/Reporting/ReportProgress.hpp"
class CalculiXJobInputBundle;

class CalculiXReportInputReader
{
public:
  CalculiXReportInputReader();
  // Resolves INCLUDE relative to its containing file. Cycles and unsupported
  // parameterized INCLUDE paths fail explicitly. Read never changes the deck.
  bool read(const ReportSource& source, std::string& error, ReportProgress* progress = nullptr);
  bool read_archive(const ReportSource& source, const CalculiXJobInputBundle& archive, std::string& error, ReportProgress* progress = nullptr);
  const QJsonArray& records() const noexcept;
  const ReportSource& resolved_source() const noexcept;

private:
  bool read_input(const ReportSource& source, const CalculiXJobInputBundle* archive, std::string& error, ReportProgress* progress);
  QJsonArray input_records;
  ReportSource input_source;
};
