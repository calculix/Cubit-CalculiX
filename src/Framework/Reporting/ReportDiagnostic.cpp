#include "ReportDiagnostic.hpp"
#include <utility>
#include "ReportJson.hpp"

ReportDiagnostic::ReportDiagnostic() = default;

QJsonObject ReportDiagnostic::to_json() const
{
  QJsonObject result;
  result.insert("section_id", QString::fromStdString(section_id));
  result.insert("message", QString::fromStdString(message));
  result.insert("severity", QString::fromStdString(severity));
  return result;
}

bool ReportDiagnostic::from_json(const QJsonObject& input, ReportDiagnostic& output, std::string& error)
{
  ReportDiagnostic candidate;
  if (!(ReportJson::text(input, "section_id", candidate.section_id, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "message", candidate.message, error)))
  {
    return false;
  }
  if (!(ReportJson::text(input, "severity", candidate.severity, error)))
  {
    return false;
  }
  output = std::move(candidate);
  error.clear();
  return true;
}
