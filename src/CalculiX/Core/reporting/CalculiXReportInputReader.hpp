#pragma once
#include "Framework/Reporting/ReportSource.hpp"
#include <QJsonArray>
class CalculiXJobInputBundle;

class CalculiXReportInputReader
{
public:
  CalculiXReportInputReader();
  // Resolves INCLUDE relative to its containing file. Cycles and unsupported
  // parameterized INCLUDE paths fail explicitly. Read never changes the deck.
  bool read(const ReportSource& source, std::string& error);
  bool read_archive(const ReportSource& source, const CalculiXJobInputBundle& archive, std::string& error);
  const QJsonArray& records() const noexcept;
  const ReportSource& resolved_source() const noexcept;

private:
  bool read_input(const ReportSource& source, const CalculiXJobInputBundle* archive, std::string& error);
  QJsonArray input_records;
  ReportSource input_source;
};
