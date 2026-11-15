#include "PostUnitsCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Framework/Interfaces/ModelEditorInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <utility>
PostUnitsCommand::PostUnitsCommand(std::weak_ptr<PostprocessorApplicationInterface> value) : application(std::move(value)) {
}
std::vector<std::string> PostUnitsCommand::get_syntax() {
  return {"post units system <string:type='quoted',number='1',label='system'>", "post units display quantity <string:type='quoted',number='1',label='quantity'> unit <string:type='quoted',number='1',label='unit'>", "post units display reset quantity <string:type='quoted',number='1',label='quantity'>", "post units defaults save", "post units {open|close}"};
}
std::vector<std::string> PostUnitsCommand::get_syntax_help() {
  return get_syntax();
}
std::vector<std::string> PostUnitsCommand::get_help() {
  return {"Set workspace presentation units without changing source numeric arrays. Open the shared Framework units editor."};
}
bool PostUnitsCommand::execute(CubitCommandData& data) {
  std::string error;
  try {
    const auto active = application.lock();
    if (!active) {
      error = "The workspace application is unavailable.";
    } else if (data.find_keyword("OPEN")) {
      if (ModelEditorInterface::open("postprocessor", "units", -1, error)) {
        return true;
      }
    } else if (data.find_keyword("CLOSE")) {
      if (ModelEditorInterface::close("postprocessor", "units", error)) {
        return true;
      }
    } else if (data.find_keyword("DEFAULTS")) {
      if (framework_units().save_defaults("postprocessor", error)) {
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
  PRINT_ERROR("Cannot configure postprocessor units: %s\n", error.c_str());
  return false;
}
