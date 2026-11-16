#pragma once
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <utility>
#include <vector>

class CalculiXUnitsInterface {
public:
  CalculiXUnitsInterface();
  bool context(SimUnits::WorkspaceUnitContext& output, std::string& error) const;
  bool set_system(const std::string& system, std::string& error) const;
  bool validate_export(std::string& error) const;
  // Validate live SDK ElementType/native-side facts, independent of export state.
  static bool validate_pressure_targets(const std::vector<std::pair<int, int>>& targets, std::string& error);
  bool input_value(double value, const std::string& quantity, const std::string& unit, double& canonical, std::string& error) const;
  bool display_value(double canonical, const std::string& quantity, double& value, std::string& unit, std::string& error) const;
  bool export_value(double canonical, const SimUnits::Dimension& dimension, double& value, std::string& error) const;
};
