#include "WorkspaceUnitContext.hpp"
#include <utility>

SimUnits::WorkspaceUnitContext::WorkspaceUnitContext() : display_units{{"mass", "kg"}, {"density", "kg/m^3"}, {"stress", "N/mm^2"}, {"pressure", "N/mm^2"}} {
}
SimUnits::WorkspaceUnitContext::WorkspaceUnitContext(std::string workspace) : WorkspaceUnitContext() {
  this->workspace = std::move(workspace);
}
