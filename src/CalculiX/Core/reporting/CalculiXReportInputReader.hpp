#pragma once
#include "Framework/Reporting/ReportSource.hpp"
#include <QJsonArray>

class CalculiXReportInputReader
{
public:
  CalculiXReportInputReader();
  // Resolves INCLUDE relative to its containing file. Cycles and unsupported
  // parameterized INCLUDE paths fail explicitly. Read never changes the deck.
  bool read(const ReportSource& source, std::string& error);
  const QJsonArray& records() const noexcept;
  const ReportSource& resolved_source() const noexcept;

private:
  QJsonArray input_records;
  ReportSource input_source;
};
