#ifndef CORELOADSPERSISTENCE_HPP
#define CORELOADSPERSISTENCE_HPP

class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreLoadsTrajectory;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;
class HDF5Tool;

class CoreLoadsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreLoadsForces& forces);
  static bool write(HDF5Tool& file, const CoreLoadsForces& forces);

  static bool read(HDF5Tool& file, CoreLoadsPressures& pressures);
  static bool write(HDF5Tool& file, const CoreLoadsPressures& pressures);

  static bool read(HDF5Tool& file, CoreLoadsHeatfluxes& heatfluxes);
  static bool write(HDF5Tool& file, const CoreLoadsHeatfluxes& heatfluxes);

  static bool read(HDF5Tool& file, CoreLoadsGravity& gravity);
  static bool write(HDF5Tool& file, const CoreLoadsGravity& gravity);

  static bool read(HDF5Tool& file, CoreLoadsCentrifugal& centrifugal);
  static bool write(HDF5Tool& file, const CoreLoadsCentrifugal& centrifugal);

  static bool read(HDF5Tool& file, CoreLoadsTrajectory& trajectory);
  static bool write(HDF5Tool& file, const CoreLoadsTrajectory& trajectory);

  static bool read(HDF5Tool& file, CoreLoadsFilm& film);
  static bool write(HDF5Tool& file, const CoreLoadsFilm& film);

  static bool read(HDF5Tool& file, CoreLoadsRadiation& radiation);
  static bool write(HDF5Tool& file, const CoreLoadsRadiation& radiation);

  static bool read(HDF5Tool& file, CoreLoadsSurfaceTraction& traction);
  static bool write(HDF5Tool& file, const CoreLoadsSurfaceTraction& traction);

  static bool read(HDF5Tool& file, CoreLoadsBodyHeatflux& bodyheatflux);
  static bool write(HDF5Tool& file, const CoreLoadsBodyHeatflux& bodyheatflux);
};

#endif
