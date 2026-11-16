#ifndef CUBITSIM_RESULTSUNITS_HPP
#define CUBITSIM_RESULTSUNITS_HPP
#include "VtkResultsData.hpp"
#include "Framework/Core/units/WorkspaceUnitContext.hpp"
namespace SimResults {
class ResultsUnits {
public:
  static bool display_value(double input, const SimUnits::QuantityUnit& source, const std::string& workspace, double& output, std::string& label, std::string& error, const SimUnits::WorkspaceUnitContext* presentation = nullptr);
  static bool display_value(double input, const SimUnits::QuantityUnit& source, const SimUnits::WorkspaceUnitContext& presentation, double& output, std::string& label, std::string& error);
  static bool field_quantity(const Field& field, const std::string& component, bool magnitude, SimUnits::QuantityUnit& output, std::string& error);
  static bool field_value(double input, const Field& field, const std::string& component, bool magnitude, const std::string& workspace, double& output, std::string& label, std::string& error, const SimUnits::WorkspaceUnitContext* presentation = nullptr);
};
}
#endif
