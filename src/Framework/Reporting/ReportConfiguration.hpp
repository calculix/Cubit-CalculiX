#pragma once
#include <QJsonObject>
#include <map>
#include <string>
#include <vector>
#include "ReportImage.hpp"
#include <optional>

class ReportConfiguration
{
public:
  ReportConfiguration();
  std::map<std::string, std::string> values;
  std::optional<ReportImage> company_logo;
  // Patch intent: false preserves the current logo; true replaces or removes it.
  // Persistence records the logo itself, never this transient flag.
  bool replace_company_logo = false;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportConfiguration& output, std::string& error);
};
