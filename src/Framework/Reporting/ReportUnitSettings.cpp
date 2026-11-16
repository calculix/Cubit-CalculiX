#include "ReportUnitSettings.hpp"
#include "ReportJson.hpp"
#include "Framework/Core/units/WorkspaceUnitContext.hpp"
#include <utility>

ReportUnitSettings::ReportUnitSettings() = default;

ReportUnitSettings ReportUnitSettings::capture(const SimUnits::WorkspaceUnitContext& context)
{
  ReportUnitSettings result;
  result.system = context.system;
  result.display_units = context.display_units;
  return result;
}

void ReportUnitSettings::apply_to(SimUnits::WorkspaceUnitContext& context) const
{
  context.system = system;
  context.display_units = display_units;
}

QJsonObject ReportUnitSettings::to_json() const
{
  return {{"system", QString::fromStdString(system)}, {"display_units", ReportJson::strings(display_units)}};
}

bool ReportUnitSettings::from_json(const QJsonObject& input, ReportUnitSettings& output, std::string& error)
{
  ReportUnitSettings candidate;
  if (!ReportJson::text(input, "system", candidate.system, error) ||
      !ReportJson::strings(input.value("display_units"), candidate.display_units, error))
  {
    return false;
  }
  if (candidate.system.empty())
  {
    error = "Report presentation units require a named system.";
    return false;
  }
  output = std::move(candidate);
  return true;
}
