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
  static void read(HDF5Tool& file, CoreLoadsForces& forces);
  static void write(HDF5Tool& file, const CoreLoadsForces& forces);

  static void read(HDF5Tool& file, CoreLoadsPressures& pressures);
  static void write(HDF5Tool& file, const CoreLoadsPressures& pressures);

  static void read(HDF5Tool& file, CoreLoadsHeatfluxes& heatfluxes);
  static void write(HDF5Tool& file, const CoreLoadsHeatfluxes& heatfluxes);

  static void read(HDF5Tool& file, CoreLoadsGravity& gravity);
  static void write(HDF5Tool& file, const CoreLoadsGravity& gravity);

  static void read(HDF5Tool& file, CoreLoadsCentrifugal& centrifugal);
  static void write(HDF5Tool& file, const CoreLoadsCentrifugal& centrifugal);

  static void read(HDF5Tool& file, CoreLoadsTrajectory& trajectory);
  static void write(HDF5Tool& file, const CoreLoadsTrajectory& trajectory);

  static void read(HDF5Tool& file, CoreLoadsFilm& film);
  static void write(HDF5Tool& file, const CoreLoadsFilm& film);

  static void read(HDF5Tool& file, CoreLoadsRadiation& radiation);
  static void write(HDF5Tool& file, const CoreLoadsRadiation& radiation);

  static void read(HDF5Tool& file, CoreLoadsSurfaceTraction& traction);
  static void write(HDF5Tool& file, const CoreLoadsSurfaceTraction& traction);

  static void read(HDF5Tool& file, CoreLoadsBodyHeatflux& bodyheatflux);
  static void write(HDF5Tool& file, const CoreLoadsBodyHeatflux& bodyheatflux);
};

#endif
