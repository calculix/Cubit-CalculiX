#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "CalculiXUnitsInterface.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "ElementType.h"
#include <algorithm>
#include <cmath>
#include <exception>

namespace {
bool validate_amplitude_axes(std::string& error) {
  if (ccx_core.steps == nullptr || ccx_core.amplitudes == nullptr) {
    return true;
  }
  const auto check = [&](int amplitude, const std::string& expected, int step) {
    if (amplitude <= 0) {
      return true;
    }
    std::string actual = "time";
    const auto axis = ccx_core.amplitudes->axis_quantities.find(amplitude);
    if (axis != ccx_core.amplitudes->axis_quantities.end()) {
      actual = axis->second;
    }
    if (actual != expected) {
      error = "Amplitude " + std::to_string(amplitude) + " uses a " + actual + " axis, but step " + std::to_string(step) + " requires " + expected + ". Define a separate curve with the appropriate axis.";
      return false;
    }
    return true;
  };
  for (const auto& step : ccx_core.steps->steps_data) {
    if (step.size() < 7) {
      error = "Incomplete step references.";
      return false;
    }
    std::string expected = "time";
    if (step[3] == 10) {
      expected = "frequency";
    }
    for (const auto& link : ccx_core.steps->loads_data) {
      if (link.size() < 3 || link[0] != step[5]) {
        continue;
      }
      const std::vector<std::vector<int>>* records = nullptr;
      if (link[1] == 1 && ccx_core.loadsforces != nullptr) {
        records = &ccx_core.loadsforces->loads_data;
      }
      if (link[1] == 2 && ccx_core.loadspressures != nullptr) {
        records = &ccx_core.loadspressures->loads_data;
      }
      if (link[1] == 3 && ccx_core.loadsheatfluxes != nullptr) {
        records = &ccx_core.loadsheatfluxes->loads_data;
      }
      if (link[1] == 4 && ccx_core.loadsgravity != nullptr) {
        records = &ccx_core.loadsgravity->loads_data;
      }
      if (link[1] == 5 && ccx_core.loadscentrifugal != nullptr) {
        records = &ccx_core.loadscentrifugal->loads_data;
      }
      if (link[1] == 7 && ccx_core.loadsfilm != nullptr) {
        records = &ccx_core.loadsfilm->loads_data;
      }
      if (link[1] == 8 && ccx_core.loadsradiation != nullptr) {
        records = &ccx_core.loadsradiation->loads_data;
      }
      if (link[1] == 9 && ccx_core.loadssurfacetraction != nullptr) {
        records = &ccx_core.loadssurfacetraction->loads_data;
      }
      if (link[1] == 10 && ccx_core.loadsbodyheatflux != nullptr) {
        records = &ccx_core.loadsbodyheatflux->loads_data;
      }
      if (records == nullptr) {
        continue;
      }
      for (const auto& record : *records) {
        if (record.size() >= 3 && record[0] == link[2]) {
          if (!check(record[2], expected, step[0])) {
            return false;
          }
          if ((link[1] == 7 || link[1] == 8) && record.size() > 7 && !check(record[7], expected, step[0])) {
            return false;
          }
        }
      }
    }
    for (const auto& link : ccx_core.steps->bcs_data) {
      if (link.size() < 3 || link[0] != step[6]) {
        continue;
      }
      const std::vector<std::vector<int>>* records = nullptr;
      if (link[1] == 1 && ccx_core.bcsdisplacements != nullptr) {
        records = &ccx_core.bcsdisplacements->bcs_data;
      } else if (link[1] == 2 && ccx_core.bcstemperatures != nullptr) {
        records = &ccx_core.bcstemperatures->bcs_data;
      }
      if (records != nullptr) {
        for (const auto& record : *records) {
          if (record.size() >= 3 && record[0] == link[2] && !check(record[2], expected, step[0])) {
            return false;
          }
        }
      }
    }
  }
  return true;
}
}

CalculiXUnitsInterface::CalculiXUnitsInterface() = default;
bool CalculiXUnitsInterface::context(SimUnits::WorkspaceUnitContext& output, std::string& error) const {
  return framework_units().workspace_context("calculix", output, error);
}
bool CalculiXUnitsInterface::set_system(const std::string& system, std::string& error) const {
  return framework_units().set_workspace_system("calculix", system, error);
}
bool CalculiXUnitsInterface::validate_export(std::string& error) const {
  try {
    error.clear();
    if (!validate_amplitude_axes(error)) {
      return false;
    }
    double geometryCheck = 0.0;
    if (framework_units().geometry_unit().empty()) {
      error = "Declare Cubit geometry length units before export, for example: ccx units geometry unit \"mm\".";
      return false;
    }
    if (!framework_units().geometry_to_si(0.0, geometryCheck, error)) {
      return false;
    }
    SimUnits::WorkspaceUnitContext current;
    if (!context(current, error)) {
      return false;
    }
    if (ccx_core.loadspressures != nullptr) {
      CalculiXMeshInterface mesh;
      for (const auto& load : ccx_core.loadspressures->loads_data) {
        if (load.size() < 5) {
          error = "Invalid pressure load target.";
          return false;
        }
        std::vector<std::pair<int, int>> targets;
        if (!mesh.get_sideset_element_sides(load[4], targets, error) ||
            !validate_pressure_targets(targets, error))
        {
          return false;
        }
      }
    }
    if (ccx_core.physicalconstants != nullptr && !ccx_core.physicalconstants->physicalconstants_data.empty()) {
      const auto& absoluteZero = ccx_core.physicalconstants->physicalconstants_data.front();
      if (!absoluteZero.empty()) {
        std::size_t consumed = 0;
        try {
          const double value = std::stod(absoluteZero, &consumed);
          if (consumed != absoluteZero.size() || value != 0.0) {
            error = "Canonical temperature export uses kelvin and requires ABSOLUTE ZERO=0.";
            return false;
          }
        } catch (const std::exception&) {
          error = "Invalid ABSOLUTE ZERO setting.";
          return false;
        }
      }
    }
    return true;
  } catch (const std::exception& exception) {
    error = std::string("Cannot validate unit-aware export: ") + exception.what();
  } catch (...) {
    error = "Cannot validate unit-aware export: unknown error.";
  }
  return false;
}

bool CalculiXUnitsInterface::validate_pressure_targets(
  const std::vector<std::pair<int, int>>& targets, std::string& error)
{
  error.clear();
  if (targets.empty())
  {
    error = "A pressure sideset must contain element sides.";
    return false;
  }
  for (const auto& target : targets)
  {
    if (target.first <= SPHERE_EXO || target.first >= INVALID_ELEMENT_TYPE || target.second < 1)
    {
      error = "Invalid pressure element type or side.";
      return false;
    }
    bool lineLoad = false;
    switch (static_cast<ElementType>(target.first))
    {
      case BAR:
      case BAR2:
      case BAR3:
      case BEAM:
      case BEAM2:
      case BEAM3:
      case TRUSS:
      case TRUSS2:
      case TRUSS3:
      case SPRING:
        lineLoad = true;
        break;
      case TRISHELL:
      case TRISHELL3:
      case TRISHELL4:
      case TRISHELL6:
      case TRISHELL7:
      case SHEL:
      case SHELL4:
      case SHELL8:
      case SHELL9:
        // Native shell sides 1 and 2 are faces; remaining sides are edges.
        lineLoad = target.second > 2;
        break;
      default:
        break;
    }
    if (lineLoad)
    {
      error = "Beam and shell-edge distributed loads require force-per-length input; the pressure command accepts force-per-area only.";
      return false;
    }
  }
  return true;
}

bool CalculiXUnitsInterface::input_value(double value, const std::string& quantity, const std::string& unit, double& canonical, std::string& error) const {
  SimUnits::QuantityUnit definition;
  std::string expression = unit;
  if (!framework_units().quantity(quantity, definition, error)) {
    return false;
  }
  if (expression.empty() && !framework_units().display_unit("calculix", quantity, expression, error)) {
    return false;
  }
  return framework_units().to_si(value, expression, definition.dimension, definition.role, canonical, error);
}
bool CalculiXUnitsInterface::display_value(double canonical, const std::string& quantity, double& value, std::string& unit, std::string& error) const {
  SimUnits::QuantityUnit definition;
  if (!framework_units().quantity(quantity, definition, error) || !framework_units().display_unit("calculix", quantity, unit, error)) {
    return false;
  }
  return framework_units().from_si(canonical, unit, definition.dimension, definition.role, value, error);
}
bool CalculiXUnitsInterface::export_value(double canonical, const SimUnits::Dimension& dimension, double& value, std::string& error) const {
  return framework_units().to_solver(canonical, dimension, "calculix", value, error);
}
