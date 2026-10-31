#ifndef CORELOADREMOVAL_HPP
#define CORELOADREMOVAL_HPP

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
class CoreSteps;

class CoreLoadRemoval
{
public:
  static bool delete_load(CoreLoadsForces& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsPressures& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsHeatfluxes& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsGravity& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsCentrifugal& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsTrajectory& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsFilm& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsRadiation& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsSurfaceTraction& loads, CoreSteps& steps, int load_id);
  static bool delete_load(CoreLoadsBodyHeatflux& loads, CoreSteps& steps, int load_id);
};

#endif
