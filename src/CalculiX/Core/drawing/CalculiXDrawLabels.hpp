#pragma once
#include "DrawLabelData.hpp"
#include "DrawLabelSettings.hpp"
#include "Framework/Drawing/DrawingGeometry.hpp"

class CalculiXDrawLabels
{
public:
  // Settings remain definitions: no unit inference or amplitude evaluation.
  // Step drawing supplies its own resolved targets and component summaries.
  static bool read(DrawEntityKey entity, DrawLabelData& output, std::string& error, bool targets = true);
  static std::string text(const DrawLabelData& data, const DrawLabelSettings& settings);
  static bool append(const DrawLabelData& data, const DrawLabelSettings& settings, double symbolSize,
    DrawingGeometry& output, std::string& error);
  static void resolved_value(DrawLabelData& data, const std::string& component, const std::string& value);
};
