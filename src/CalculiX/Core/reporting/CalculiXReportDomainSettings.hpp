#pragma once
#include <string>
#include <memory>
struct SectionsSettings;
struct OrientationsSettings;
struct ConstraintsSettings;
struct ContactPairsSettings;
struct SurfaceInteractionsSettings;
struct StepsSettings;
struct InitialConditionsSettings;
struct DampingSettings;
struct PhysicalConstantsSettings;
struct AmplitudesSettings;
struct LoadsForcesSettings;
struct LoadsPressuresSettings;
struct LoadsHeatfluxesSettings;
struct LoadsGravitySettings;
struct LoadsCentrifugalSettings;
struct LoadsFilmSettings;
struct LoadsRadiationSettings;
struct LoadsSurfaceTractionSettings;
struct LoadsBodyHeatfluxSettings;
struct BCsDisplacementsSettings;
struct BCsTemperaturesSettings;
struct LoadsTrajectorySettings;
class CoreSections;
class CoreOrientations;
class CoreConstraints;
class CoreContactPairs;
class CoreSurfaceInteractions;
class CoreSteps;
class CoreInitialConditions;
class CoreDamping;
class CorePhysicalConstants;
class CoreAmplitudes;
class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreLoadsTrajectory;
class CoreLoadsTrajectoryHeatflux;
class CoreLoadsTrajectoryBodyHeatfluxSphere;

// One collection pass owns these lazily converted views. Never retain globally.
class CalculiXReportDomainSettings
{
public:
  CalculiXReportDomainSettings();
  ~CalculiXReportDomainSettings();
  bool get(int id, SectionsSettings& settings, std::string& error);
  bool get(int id, OrientationsSettings& settings, std::string& error);
  bool get(int id, ConstraintsSettings& settings, std::string& error);
  bool get(int id, ContactPairsSettings& settings, std::string& error);
  bool get(int id, SurfaceInteractionsSettings& settings, std::string& error);
  bool get(int id, StepsSettings& settings, std::string& error);
  bool get(int id, InitialConditionsSettings& settings, std::string& error);
  bool get(int id, DampingSettings& settings, std::string& error);
  bool get(int id, PhysicalConstantsSettings& settings, std::string& error);
  bool get(int id, AmplitudesSettings& settings, std::string& error);
  bool get(int id, LoadsForcesSettings& settings, std::string& error);
  bool get(int id, LoadsPressuresSettings& settings, std::string& error);
  bool get(int id, LoadsHeatfluxesSettings& settings, std::string& error);
  bool get(int id, LoadsGravitySettings& settings, std::string& error);
  bool get(int id, LoadsCentrifugalSettings& settings, std::string& error);
  bool get(int id, LoadsFilmSettings& settings, std::string& error);
  bool get(int id, LoadsRadiationSettings& settings, std::string& error);
  bool get(int id, LoadsSurfaceTractionSettings& settings, std::string& error);
  bool get(int id, LoadsBodyHeatfluxSettings& settings, std::string& error);
  bool get(int id, BCsDisplacementsSettings& settings, std::string& error);
  bool get(int id, BCsTemperaturesSettings& settings, std::string& error);
  bool get(int id, LoadsTrajectorySettings& settings, std::string& error);

private:
  std::unique_ptr<CoreSections> sectionsView;
  std::unique_ptr<CoreOrientations> orientationsView;
  std::unique_ptr<CoreConstraints> constraintsView;
  std::unique_ptr<CoreContactPairs> contactpairsView;
  std::unique_ptr<CoreSurfaceInteractions> surfaceinteractionsView;
  std::unique_ptr<CoreSteps> stepsView;
  std::unique_ptr<CoreInitialConditions> initialconditionsView;
  std::unique_ptr<CoreDamping> dampingView;
  std::unique_ptr<CorePhysicalConstants> physicalconstantsView;
  std::unique_ptr<CoreAmplitudes> amplitudesView;
  std::unique_ptr<CoreLoadsForces> loadsforcesView;
  std::unique_ptr<CoreLoadsPressures> loadspressuresView;
  std::unique_ptr<CoreLoadsHeatfluxes> loadsheatfluxesView;
  std::unique_ptr<CoreLoadsGravity> loadsgravityView;
  std::unique_ptr<CoreLoadsCentrifugal> loadscentrifugalView;
  std::unique_ptr<CoreLoadsFilm> loadsfilmView;
  std::unique_ptr<CoreLoadsRadiation> loadsradiationView;
  std::unique_ptr<CoreLoadsSurfaceTraction> loadssurfacetractionView;
  std::unique_ptr<CoreLoadsBodyHeatflux> loadsbodyheatfluxView;
  std::unique_ptr<CoreBCsDisplacements> bcsdisplacementsView;
  std::unique_ptr<CoreBCsTemperatures> bcstemperaturesView;
  // Children own the storage borrowed by the detached trajectory parent.
  std::unique_ptr<CoreLoadsTrajectoryHeatflux> trajectoryHeatView;
  std::unique_ptr<CoreLoadsTrajectoryBodyHeatfluxSphere> trajectoryBodyView;
  std::unique_ptr<CoreLoadsTrajectory> trajectoryView;
};
