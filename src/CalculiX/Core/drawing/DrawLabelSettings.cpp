#include "DrawLabelSettings.hpp"
#include <cmath>
#include <set>
#include <utility>

DrawLabelSettings::DrawLabelSettings()
  : enabled(true), loads(true), boundaryConditions(true), selectedOnly(false), showId(true), showName(true),
    showValues(false), showAmplitude(false), leaderLines(true), offsetFactor(0.75), color("black")
{
}

bool DrawLabelSettings::validate(std::string& error) const
{
  const std::set<std::string> colors = {"red", "yellow", "green", "blue", "cyan", "magenta", "white", "black", "orange"};
  if (!std::isfinite(offsetFactor) || offsetFactor < 0 || offsetFactor > 100 || colors.count(color) == 0)
  {
    error = "Label offset must be finite and between 0 and 100; choose a supported label color.";
    return false;
  }
  error.clear();
  return true;
}

QJsonObject DrawLabelSettings::to_json() const
{
  return {{"enabled", enabled}, {"loads", loads}, {"bcs", boundaryConditions}, {"selected_only", selectedOnly},
    {"ids", showId}, {"names", showName}, {"values", showValues}, {"amplitudes", showAmplitude},
    {"leaders", leaderLines}, {"offset", offsetFactor}, {"color", QString::fromStdString(color)}};
}

bool DrawLabelSettings::from_json(const QJsonObject& input, DrawLabelSettings& output, std::string& error)
{
  DrawLabelSettings candidate;
  for (const auto& field : {std::pair<const char*, bool*>{"enabled", &candidate.enabled}, {"loads", &candidate.loads},
      {"bcs", &candidate.boundaryConditions}, {"selected_only", &candidate.selectedOnly}, {"ids", &candidate.showId},
      {"names", &candidate.showName}, {"values", &candidate.showValues}, {"amplitudes", &candidate.showAmplitude}, {"leaders", &candidate.leaderLines}})
  {
    if (!input.value(field.first).isBool())
    {
      error = std::string("Missing or invalid label setting: ") + field.first;
      return false;
    }
    *field.second = input.value(field.first).toBool();
  }
  if (!input.value("offset").isDouble() || !input.value("color").isString())
  {
    error = "Missing label offset or color.";
    return false;
  }
  candidate.offsetFactor = input.value("offset").toDouble();
  candidate.color = input.value("color").toString().toStdString();
  if (!candidate.validate(error))
  {
    return false;
  }
  output = std::move(candidate);
  return true;
}
