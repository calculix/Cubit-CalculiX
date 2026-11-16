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

#include <utility>

CalculiXReportDomainSettings::CalculiXReportDomainSettings() = default;
CalculiXReportDomainSettings::~CalculiXReportDomainSettings() = default;

bool CalculiXReportDomainSettings::get(int id, SectionsSettings& settings, std::string& error)
{
  if (!sectionsView)
  {
    if (ccx_core.sections == nullptr)
    {
      error = "CalculiX section storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreSections>(*ccx_core.sections);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    sectionsView = std::move(candidate);
  }
  return sectionsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, OrientationsSettings& settings, std::string& error)
{
  if (!orientationsView)
  {
    if (ccx_core.orientations == nullptr)
    {
      error = "CalculiX orientation storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreOrientations>(*ccx_core.orientations);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    orientationsView = std::move(candidate);
  }
  return orientationsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, ConstraintsSettings& settings, std::string& error)
{
  if (!constraintsView)
  {
    if (ccx_core.constraints == nullptr)
    {
      error = "CalculiX constraint storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreConstraints>(*ccx_core.constraints);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    constraintsView = std::move(candidate);
  }
  return constraintsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, ContactPairsSettings& settings, std::string& error)
{
  if (!contactpairsView)
  {
    if (ccx_core.contactpairs == nullptr)
    {
      error = "CalculiX contactpair storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreContactPairs>(*ccx_core.contactpairs);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    contactpairsView = std::move(candidate);
  }
  return contactpairsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, SurfaceInteractionsSettings& settings, std::string& error)
{
  if (!surfaceinteractionsView)
  {
    if (ccx_core.surfaceinteractions == nullptr)
    {
      error = "CalculiX surfaceinteraction storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreSurfaceInteractions>(*ccx_core.surfaceinteractions);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    surfaceinteractionsView = std::move(candidate);
  }
  return surfaceinteractionsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, StepsSettings& settings, std::string& error)
{
  if (!stepsView)
  {
    if (ccx_core.steps == nullptr)
    {
      error = "CalculiX step storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreSteps>(*ccx_core.steps);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    stepsView = std::move(candidate);
  }
  return stepsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, InitialConditionsSettings& settings, std::string& error)
{
  if (!initialconditionsView)
  {
    if (ccx_core.initialconditions == nullptr)
    {
      error = "CalculiX initialcondition storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreInitialConditions>(*ccx_core.initialconditions);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    initialconditionsView = std::move(candidate);
  }
  return initialconditionsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, DampingSettings& settings, std::string& error)
{
  if (!dampingView)
  {
    if (ccx_core.damping == nullptr)
    {
      error = "CalculiX damping storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreDamping>(*ccx_core.damping);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    dampingView = std::move(candidate);
  }
  return dampingView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, PhysicalConstantsSettings& settings, std::string& error)
{
  if (!physicalconstantsView)
  {
    if (ccx_core.physicalconstants == nullptr)
    {
      error = "CalculiX physicalconstants storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CorePhysicalConstants>(*ccx_core.physicalconstants);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    physicalconstantsView = std::move(candidate);
  }
  return physicalconstantsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, AmplitudesSettings& settings, std::string& error)
{
  if (!amplitudesView)
  {
    if (ccx_core.amplitudes == nullptr)
    {
      error = "CalculiX amplitude storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreAmplitudes>(*ccx_core.amplitudes);
    if (!CalculiXDomainUnits::convert(*candidate, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    amplitudesView = std::move(candidate);
  }
  return amplitudesView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsForcesSettings& settings, std::string& error)
{
  if (!loadsforcesView)
  {
    if (ccx_core.loadsforces == nullptr)
    {
      error = "CalculiX force storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsForces>(*ccx_core.loadsforces);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadsforcesView = std::move(candidate);
  }
  return loadsforcesView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsPressuresSettings& settings, std::string& error)
{
  if (!loadspressuresView)
  {
    if (ccx_core.loadspressures == nullptr)
    {
      error = "CalculiX pressure storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsPressures>(*ccx_core.loadspressures);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadspressuresView = std::move(candidate);
  }
  return loadspressuresView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsHeatfluxesSettings& settings, std::string& error)
{
  if (!loadsheatfluxesView)
  {
    if (ccx_core.loadsheatfluxes == nullptr)
    {
      error = "CalculiX heatflux storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsHeatfluxes>(*ccx_core.loadsheatfluxes);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadsheatfluxesView = std::move(candidate);
  }
  return loadsheatfluxesView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsGravitySettings& settings, std::string& error)
{
  if (!loadsgravityView)
  {
    if (ccx_core.loadsgravity == nullptr)
    {
      error = "CalculiX gravity storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsGravity>(*ccx_core.loadsgravity);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadsgravityView = std::move(candidate);
  }
  return loadsgravityView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsCentrifugalSettings& settings, std::string& error)
{
  if (!loadscentrifugalView)
  {
    if (ccx_core.loadscentrifugal == nullptr)
    {
      error = "CalculiX centrifugal storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsCentrifugal>(*ccx_core.loadscentrifugal);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadscentrifugalView = std::move(candidate);
  }
  return loadscentrifugalView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsFilmSettings& settings, std::string& error)
{
  if (!loadsfilmView)
  {
    if (ccx_core.loadsfilm == nullptr)
    {
      error = "CalculiX film storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsFilm>(*ccx_core.loadsfilm);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadsfilmView = std::move(candidate);
  }
  return loadsfilmView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsRadiationSettings& settings, std::string& error)
{
  if (!loadsradiationView)
  {
    if (ccx_core.loadsradiation == nullptr)
    {
      error = "CalculiX radiation storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsRadiation>(*ccx_core.loadsradiation);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadsradiationView = std::move(candidate);
  }
  return loadsradiationView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsSurfaceTractionSettings& settings, std::string& error)
{
  if (!loadssurfacetractionView)
  {
    if (ccx_core.loadssurfacetraction == nullptr)
    {
      error = "CalculiX surfacetraction storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsSurfaceTraction>(*ccx_core.loadssurfacetraction);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadssurfacetractionView = std::move(candidate);
  }
  return loadssurfacetractionView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsBodyHeatfluxSettings& settings, std::string& error)
{
  if (!loadsbodyheatfluxView)
  {
    if (ccx_core.loadsbodyheatflux == nullptr)
    {
      error = "CalculiX bodyheatflux storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsBodyHeatflux>(*ccx_core.loadsbodyheatflux);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    loadsbodyheatfluxView = std::move(candidate);
  }
  return loadsbodyheatfluxView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, BCsDisplacementsSettings& settings, std::string& error)
{
  if (!bcsdisplacementsView)
  {
    if (ccx_core.bcsdisplacements == nullptr)
    {
      error = "CalculiX displacement storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreBCsDisplacements>(*ccx_core.bcsdisplacements);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    bcsdisplacementsView = std::move(candidate);
  }
  return bcsdisplacementsView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, BCsTemperaturesSettings& settings, std::string& error)
{
  if (!bcstemperaturesView)
  {
    if (ccx_core.bcstemperatures == nullptr)
    {
      error = "CalculiX temperature storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreBCsTemperatures>(*ccx_core.bcstemperatures);
    if (!CalculiXPhysicalModelUnits::convert(*candidate, CalculiXUnitView::Report, error))
    {
      return false;
    }
    bcstemperaturesView = std::move(candidate);
  }
  return bcstemperaturesView->get_settings(id, settings, error);
}

bool CalculiXReportDomainSettings::get(int id, LoadsTrajectorySettings& settings, std::string& error)
{
  if (!trajectoryView)
  {
    if (ccx_core.loadstrajectory == nullptr || ccx_core.loadstrajectory->heatflux == nullptr ||
        ccx_core.loadstrajectory->bodyheatfluxsphere == nullptr)
    {
      error = "CalculiX trajectory storage is unavailable.";
      return false;
    }
    auto candidate = std::make_unique<CoreLoadsTrajectory>(*ccx_core.loadstrajectory);
    auto heat = std::make_unique<CoreLoadsTrajectoryHeatflux>(*candidate->heatflux);
    auto body = std::make_unique<CoreLoadsTrajectoryBodyHeatfluxSphere>(*candidate->bodyheatfluxsphere);
    if (!CalculiXDomainUnits::convert(*heat, CalculiXDomainUnitView::Report, error) ||
        !CalculiXDomainUnits::convert(*body, CalculiXDomainUnitView::Report, error))
    {
      return false;
    }
    candidate->domain_unit_view = true;
    candidate->heatflux = heat.get();
    candidate->bodyheatfluxsphere = body.get();
    trajectoryHeatView = std::move(heat);
    trajectoryBodyView = std::move(body);
    trajectoryView = std::move(candidate);
  }
  return trajectoryView->get_settings(id, settings, error);
}
