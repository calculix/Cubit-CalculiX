#pragma once

#include <QJsonObject>
#include <map>
#include <string>

namespace SimUnits
{
struct WorkspaceUnitContext;
}

class ReportUnitSettings
{
public:
  ReportUnitSettings();
  std::string system;
  std::map<std::string, std::string> display_units;

  static ReportUnitSettings capture(const SimUnits::WorkspaceUnitContext& context);
  // Changes presentation only; workspace identity and native solver units survive.
  void apply_to(SimUnits::WorkspaceUnitContext& context) const;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, ReportUnitSettings& output, std::string& error);
};
