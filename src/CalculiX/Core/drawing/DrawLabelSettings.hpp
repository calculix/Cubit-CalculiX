#pragma once
#include <QJsonObject>
#include <string>

class DrawLabelSettings
{
public:
  DrawLabelSettings();
  bool enabled;
  bool loads;
  bool boundaryConditions;
  bool selectedOnly;
  bool showId;
  bool showName;
  bool showValues;
  bool showAmplitude;
  bool leaderLines;
  double offsetFactor;
  std::string color;
  bool validate(std::string& error) const;
  QJsonObject to_json() const;
  static bool from_json(const QJsonObject& input, DrawLabelSettings& output, std::string& error);
};
