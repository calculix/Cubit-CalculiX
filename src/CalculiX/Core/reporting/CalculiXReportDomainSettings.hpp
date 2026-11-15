#pragma once
#include <string>
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
class CalculiXReportDomainSettings {
public:
  static bool get(int id, SectionsSettings& settings, std::string& error);
  static bool get(int id, OrientationsSettings& settings, std::string& error);
  static bool get(int id, ConstraintsSettings& settings, std::string& error);
  static bool get(int id, ContactPairsSettings& settings, std::string& error);
  static bool get(int id, SurfaceInteractionsSettings& settings, std::string& error);
  static bool get(int id, StepsSettings& settings, std::string& error);
  static bool get(int id, InitialConditionsSettings& settings, std::string& error);
  static bool get(int id, DampingSettings& settings, std::string& error);
  static bool get(int id, PhysicalConstantsSettings& settings, std::string& error);
  static bool get(int id, AmplitudesSettings& settings, std::string& error);
  static bool get(int id, LoadsForcesSettings& settings, std::string& error);
  static bool get(int id, LoadsPressuresSettings& settings, std::string& error);
  static bool get(int id, LoadsHeatfluxesSettings& settings, std::string& error);
  static bool get(int id, LoadsGravitySettings& settings, std::string& error);
  static bool get(int id, LoadsCentrifugalSettings& settings, std::string& error);
  static bool get(int id, LoadsFilmSettings& settings, std::string& error);
  static bool get(int id, LoadsRadiationSettings& settings, std::string& error);
  static bool get(int id, LoadsSurfaceTractionSettings& settings, std::string& error);
  static bool get(int id, LoadsBodyHeatfluxSettings& settings, std::string& error);
  static bool get(int id, BCsDisplacementsSettings& settings, std::string& error);
  static bool get(int id, BCsTemperaturesSettings& settings, std::string& error);
  static bool get(int id, LoadsTrajectorySettings& settings, std::string& error);
};
