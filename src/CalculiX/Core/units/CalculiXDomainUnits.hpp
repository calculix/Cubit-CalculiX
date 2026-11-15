#pragma once
#include <string>
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
class CoreLoadsTrajectoryHeatflux;
class CoreLoadsTrajectoryBodyHeatfluxSphere;
enum class CalculiXDomainUnitView { Input, Native, Display, Solver, Geometry, Report };

class CalculiXDomainUnits {
public:
  static bool convert(CoreSections& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreOrientations& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreConstraints& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreContactPairs& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreSurfaceInteractions& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreSteps& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreInitialConditions& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreDamping& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CorePhysicalConstants& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreAmplitudes& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreLoadsTrajectoryHeatflux& model, CalculiXDomainUnitView view, std::string& error);
  static bool convert(CoreLoadsTrajectoryBodyHeatfluxSphere& model, CalculiXDomainUnitView view, std::string& error);
};
