#pragma once
#include "ReportSource.hpp"
#include "ReportConfiguration.hpp"
#include "ReportSection.hpp"
#include "ReportDiagnostic.hpp"
#include <QJsonObject>
#include <map>

class ReportDocument
{
public:
  ReportDocument();
  std::string id;
  std::string title;
  std::string provider;
  std::string template_id;
  int template_revision = 0;
  std::string profile_id;
  int profile_revision = 0;
  std::string generated_at;
  ReportSource source;
  ReportConfiguration configuration;
  std::vector<ReportSection> sections;
  std::vector<ReportDiagnostic> diagnostics;
  // Stable section IDs keep user notes separate from regenerated provider content.
  std::map<std::string, std::string> notes;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportDocument& output, std::string& error);
  bool validate(std::string& error) const;
};
