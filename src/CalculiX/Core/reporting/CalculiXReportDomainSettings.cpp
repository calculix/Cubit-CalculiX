#include "CalculiXReportDomainSettings.hpp"
#include "Core/loadCalculiXCore.hpp"
#include "Core/units/CalculiXDomainUnits.hpp"
#include "Core/units/CalculiXPhysicalModelUnits.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"

bool CalculiXReportDomainSettings::get(int id, SectionsSettings& settings, std::string& error) {
  if (ccx_core.sections == nullptr) {
    error = "CalculiX section storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.sections;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, OrientationsSettings& settings, std::string& error) {
  if (ccx_core.orientations == nullptr) {
    error = "CalculiX orientation storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.orientations;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, ConstraintsSettings& settings, std::string& error) {
  if (ccx_core.constraints == nullptr) {
    error = "CalculiX constraint storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.constraints;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, ContactPairsSettings& settings, std::string& error) {
  if (ccx_core.contactpairs == nullptr) {
    error = "CalculiX contactpair storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.contactpairs;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, SurfaceInteractionsSettings& settings, std::string& error) {
  if (ccx_core.surfaceinteractions == nullptr) {
    error = "CalculiX surfaceinteraction storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.surfaceinteractions;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, StepsSettings& settings, std::string& error) {
  if (ccx_core.steps == nullptr) {
    error = "CalculiX step storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.steps;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, InitialConditionsSettings& settings, std::string& error) {
  if (ccx_core.initialconditions == nullptr) {
    error = "CalculiX initialcondition storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.initialconditions;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, DampingSettings& settings, std::string& error) {
  if (ccx_core.damping == nullptr) {
    error = "CalculiX damping storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.damping;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, PhysicalConstantsSettings& settings, std::string& error) {
  if (ccx_core.physicalconstants == nullptr) {
    error = "CalculiX physicalconstants storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.physicalconstants;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, AmplitudesSettings& settings, std::string& error) {
  if (ccx_core.amplitudes == nullptr) {
    error = "CalculiX amplitude storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.amplitudes;
  if (!CalculiXDomainUnits::convert(candidate, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsForcesSettings& settings, std::string& error) {
  if (ccx_core.loadsforces == nullptr) {
    error = "CalculiX force storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadsforces;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsPressuresSettings& settings, std::string& error) {
  if (ccx_core.loadspressures == nullptr) {
    error = "CalculiX pressure storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadspressures;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsHeatfluxesSettings& settings, std::string& error) {
  if (ccx_core.loadsheatfluxes == nullptr) {
    error = "CalculiX heatflux storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadsheatfluxes;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsGravitySettings& settings, std::string& error) {
  if (ccx_core.loadsgravity == nullptr) {
    error = "CalculiX gravity storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadsgravity;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsCentrifugalSettings& settings, std::string& error) {
  if (ccx_core.loadscentrifugal == nullptr) {
    error = "CalculiX centrifugal storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadscentrifugal;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsFilmSettings& settings, std::string& error) {
  if (ccx_core.loadsfilm == nullptr) {
    error = "CalculiX film storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadsfilm;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsRadiationSettings& settings, std::string& error) {
  if (ccx_core.loadsradiation == nullptr) {
    error = "CalculiX radiation storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadsradiation;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsSurfaceTractionSettings& settings, std::string& error) {
  if (ccx_core.loadssurfacetraction == nullptr) {
    error = "CalculiX surfacetraction storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadssurfacetraction;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsBodyHeatfluxSettings& settings, std::string& error) {
  if (ccx_core.loadsbodyheatflux == nullptr) {
    error = "CalculiX bodyheatflux storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadsbodyheatflux;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, BCsDisplacementsSettings& settings, std::string& error) {
  if (ccx_core.bcsdisplacements == nullptr) {
    error = "CalculiX displacement storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.bcsdisplacements;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, BCsTemperaturesSettings& settings, std::string& error) {
  if (ccx_core.bcstemperatures == nullptr) {
    error = "CalculiX temperature storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.bcstemperatures;
  if (!CalculiXPhysicalModelUnits::convert(candidate, CalculiXUnitView::Report, error)) {
    return false;
  }
  return candidate.get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsTrajectorySettings& settings, std::string& error) {
  if (ccx_core.loadstrajectory == nullptr || ccx_core.loadstrajectory->heatflux == nullptr || ccx_core.loadstrajectory->bodyheatfluxsphere == nullptr) {
    error = "CalculiX trajectory storage is unavailable.";
    return false;
  }
  auto candidate = *ccx_core.loadstrajectory;
  auto heat = *candidate.heatflux;
  auto body = *candidate.bodyheatfluxsphere;
  if (!CalculiXDomainUnits::convert(heat, CalculiXDomainUnitView::Report, error) || !CalculiXDomainUnits::convert(body, CalculiXDomainUnitView::Report, error)) {
    return false;
  }
  candidate.domain_unit_view = true;
  candidate.heatflux = &heat;
  candidate.bodyheatfluxsphere = &body;
  return candidate.get_settings(id, settings, error);
}
