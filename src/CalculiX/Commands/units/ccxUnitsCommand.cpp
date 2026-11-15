#include "Core/application/CalculiXApplicationInterface.hpp"
#include "ccxUnitsCommand.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Core/units/CalculiXUnitsInterface.hpp"
#include "Framework/Interfaces/ModelEditorInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>

ccxUnitsCommand::ccxUnitsCommand() = default;
std::vector<std::string> ccxUnitsCommand::get_syntax() {
  return {
    "ccx units list",
    "ccx units system <string:type='quoted',number='1',label='unit_system'>",
    "ccx units display quantity <string:type='quoted',number='1',label='quantity'> unit <string:type='quoted',number='1',label='unit'>",
    "ccx units display reset quantity <string:type='quoted',number='1',label='quantity'>",
    "ccx units geometry unit <string:type='quoted',number='1',label='unit'>",
    "ccx units defaults save",
    "ccx units convert <value:label='value'> from <string:type='quoted',number='1',label='from_unit'> to <string:type='quoted',number='1',label='to_unit'>",
    "ccx units open",
    "ccx units close"
  };
}
std::vector<std::string> ccxUnitsCommand::get_syntax_help() {
  return {"ccx units {list|system \"N-mm-s\"|open|close|defaults save}", "ccx units display quantity \"density\" unit \"kg/m^3\"", "ccx units display reset quantity \"density\"", "ccx units geometry unit \"mm\"", "ccx units convert 210 from \"GPa\" to \"N/mm^2\""};
}
std::vector<std::string> ccxUnitsCommand::get_help() {
  return {"Select CalculiX workspace units independently from other workspaces. Presets: SI, N-mm-s, kN-m-s, N-mm-ms, kN-mm-ms.", "Material data is canonical SI. Display preferences do not change coherent solver export units.", "Geometry declares the physical length of existing Cubit coordinates and never rescales geometry.", "Save defaults affects newly initialized projects. Convert prints a scalar conversion without changing model data."};
}
bool ccxUnitsCommand::execute(CubitCommandData& data) {
  try {
    auto& units = framework_units();
    std::string error;
    bool accepted = false;
    if (data.find_keyword("LIST")) {
      SimUnits::WorkspaceUnitContext context;
      if (!units.workspace_context("calculix", context, error)) {
        PRINT_ERROR("Cannot read CalculiX units: %s\n", error.c_str());
        return false;
      }
      PRINT_INFO("ccx units system \"%s\"\n", context.system.c_str());
      for (const auto& item : context.display_units) {
        PRINT_INFO("ccx units display quantity \"%s\" unit \"%s\"\n", item.first.c_str(), item.second.c_str());
      }
      if (!units.geometry_unit().empty()) {
        PRINT_INFO("ccx units geometry unit \"%s\"\n", units.geometry_unit().c_str());
      }
      return true;
    } else if (data.find_keyword("OPEN")) {
      accepted = ModelEditorInterface::open("calculix", "units", -1, error);
    } else if (data.find_keyword("CLOSE")) {
      accepted = ModelEditorInterface::close("calculix", "units", error);
    } else if (data.find_keyword("DEFAULTS")) {
      accepted = units.save_defaults("calculix", error);
    } else if (data.find_keyword("SYSTEM")) {
      std::string system;
      accepted = data.get_string("unit_system", system) && CalculiXUnitsInterface().set_system(system, error);
    } else if (data.find_keyword("DISPLAY")) {
      std::string quantity;
      std::string expression;
      if (!data.get_string("quantity", quantity)) {
        error = "A quantity is required.";
      } else if (data.find_keyword("RESET")) {
        accepted = units.set_display_unit("calculix", quantity, {}, error);
      } else {
        accepted = data.get_string("unit", expression) && units.set_display_unit("calculix", quantity, expression, error);
      }
    } else if (data.find_keyword("GEOMETRY")) {
      std::string expression;
      accepted = data.get_string("unit", expression) && units.declare_geometry_unit(expression, error);
    } else if (data.find_keyword("CONVERT")) {
      double value = 0.0;
      double converted = 0.0;
      std::string source;
      std::string target;
      accepted = data.get_value("value", value) && data.get_string("from_unit", source) && data.get_string("to_unit", target) && units.convert(value, source, target, converted, error);
      if (accepted) {
        PRINT_INFO("%.17g %s = %.17g %s\n", value, source.c_str(), converted, target.c_str());
      }
    }
    if (!accepted) {
      if (error.empty()) {
        error = "Missing or invalid unit command arguments.";
      }
      PRINT_ERROR("Cannot apply CalculiX units: %s\n", error.c_str());
    }
    if (accepted) {
      CalculiXApplicationInterface().set_gui_updated(false);
    }
    return accepted;
  } catch (const std::exception& exception) {
    PRINT_ERROR("Cannot apply CalculiX units: %s\n", exception.what());
  } catch (...) {
    PRINT_ERROR("Cannot apply CalculiX units: unknown error.\n");
  }
  return false;
}
