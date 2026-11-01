#ifndef CORETOPOLOGYDEFAULTPRESERVATION_HPP
#define CORETOPOLOGYDEFAULTPRESERVATION_HPP

#include <map>
#include <set>

class CoreSteps;
class CoreConstraints;
class CoreContactPairs;
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
class MeshExportInterface;
class CoreBlocks;
class CoreLoadsTrajectory;

struct CoreTopologyPreservationData
{
  CoreSteps& steps;
  CoreConstraints& constraints;
  CoreContactPairs& contacts;
  CoreLoadsForces& forces;
  CoreLoadsPressures& pressures;
  CoreLoadsHeatfluxes& heatfluxes;
  CoreLoadsGravity& gravity;
  CoreLoadsCentrifugal& centrifugal;
  CoreLoadsFilm& film;
  CoreLoadsRadiation& radiation;
  CoreLoadsSurfaceTraction& traction;
  CoreLoadsBodyHeatflux& bodyheatflux;
  CoreBCsDisplacements& displacements;
  CoreBCsTemperatures& temperatures;
  MeshExportInterface& mesh;
  CoreBlocks* blocks;
  CoreLoadsTrajectory* trajectories;
};

class CoreTopologyDefaultPreservation
{
public:
  static bool collect(const CoreTopologyPreservationData& data, std::map<int, std::set<int>>& default_sets, std::set<int>& default_elements);
};

#endif
