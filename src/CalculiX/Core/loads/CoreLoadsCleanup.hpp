#ifndef CORELOADSCLEANUP_HPP
#define CORELOADSCLEANUP_HPP

#include <string>

class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreBlocks;
class CoreLoadsTrajectory;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;
class CoreAmplitudes;
class CoreSteps;
class MeshExportInterface;

struct CoreLoadsCleanupData
{
  CoreLoadsForces& forces;
  CoreLoadsPressures& pressures;
  CoreLoadsHeatfluxes& heatfluxes;
  CoreLoadsGravity& gravity;
  CoreLoadsCentrifugal& centrifugal;
  CoreLoadsTrajectory& trajectory;
  CoreLoadsFilm& film;
  CoreLoadsRadiation& radiation;
  CoreLoadsSurfaceTraction& traction;
  CoreLoadsBodyHeatflux& bodyheatflux;
  CoreAmplitudes& amplitudes;
  CoreSteps& steps;
  CoreBlocks& blocks;
  MeshExportInterface& mesh;
};

class CoreLoadsCleanup
{
public:
  static bool cleanup(const CoreLoadsCleanupData& data, std::string& log);
  static bool cleanup(CoreLoadsForces& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log);
  static bool cleanup(CoreLoadsPressures& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log);
  static bool cleanup(CoreLoadsHeatfluxes& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log);

  static bool cleanup(CoreLoadsGravity& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, CoreBlocks& blocks, std::string& log);
  static bool cleanup(CoreLoadsCentrifugal& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, CoreBlocks& blocks, std::string& log);

  static bool cleanup(CoreLoadsTrajectory& loads, CoreSteps& steps, std::string& log);

  static bool cleanup(CoreLoadsFilm& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log);
  static bool cleanup(CoreLoadsRadiation& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log);

  static bool cleanup(CoreLoadsSurfaceTraction& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, MeshExportInterface& mesh, std::string& log);
  static bool cleanup(CoreLoadsBodyHeatflux& loads, CoreAmplitudes& amplitudes, CoreSteps& steps, CoreBlocks& blocks, std::string& log);
};

#endif
