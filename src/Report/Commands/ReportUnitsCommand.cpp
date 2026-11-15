#include "ReportUnitsCommand.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Interfaces/ModelEditorInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
ReportUnitsCommand::ReportUnitsCommand(std::weak_ptr<ReportApplicationInterface> value) : application(std::move(value)) {
}
std::vector<std::string> ReportUnitsCommand::get_syntax() {
  return {"report units system <string:type='quoted',number='1',label='system'>", "report units display quantity <string:type='quoted',number='1',label='quantity'> unit <string:type='quoted',number='1',label='unit'>", "report units display reset quantity <string:type='quoted',number='1',label='quantity'>", "report units defaults save", "report units {open|close}"};
}
std::vector<std::string> ReportUnitsCommand::get_syntax_help() {
  return get_syntax();
}
std::vector<std::string> ReportUnitsCommand::get_help() {
  return {"Set workspace presentation units without changing source numeric arrays. Open the shared Framework units editor."};
}
bool ReportUnitsCommand::execute(CubitCommandData& data) {
  std::string error;
  try {
    const auto active = application.lock();
    if (!active) {
      error = "The workspace application is unavailable.";
    } else if (data.find_keyword("OPEN")) {
      if (ModelEditorInterface::open("report", "units", -1, error)) {
        return true;
      }
    } else if (data.find_keyword("CLOSE")) {
      if (ModelEditorInterface::close("report", "units", error)) {
        return true;
      }
    } else if (data.find_keyword("DEFAULTS")) {
      if (framework_units().save_defaults("report", error)) {
        return true;
      }
    } else {
      std::string system;
      std::string quantity;
      std::string unit;
      bool valid = false;
      if (data.find_keyword("SYSTEM")) {
        valid = data.get_string("system", system) && !system.empty();
      } else if (data.get_string("quantity", quantity) && !quantity.empty()) {
        valid = data.find_keyword("RESET") || (data.get_string("unit", unit) && !unit.empty());
      }
      if (valid && active->configure_units(system, quantity, unit, error)) {
        return true;
      }
    }
  } catch (const std::exception& exception) {
    error = exception.what();
  } catch (...) {
    error = "Unknown unit configuration failure.";
  }
  if (error.empty()) {
    error = "Invalid units command arguments.";
  }
  PRINT_ERROR("Cannot configure report units: %s\n", error.c_str());
  return false;
}
