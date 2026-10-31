#ifndef CORELOADSTREEDATA_HPP
#define CORELOADSTREEDATA_HPP

#include <string>
#include <vector>

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

class CoreLoadsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_loadsforces_tree_data(CoreLoadsForces& loads);
  static std::vector<std::vector<std::string>> get_loadspressures_tree_data(CoreLoadsPressures& loads);
  static std::vector<std::vector<std::string>> get_loadsheatfluxes_tree_data(CoreLoadsHeatfluxes& loads);
  static std::vector<std::vector<std::string>> get_loadsgravity_tree_data(CoreLoadsGravity& loads);
  static std::vector<std::vector<std::string>> get_loadscentrifugal_tree_data(CoreLoadsCentrifugal& loads);
  static std::vector<std::vector<std::string>> get_loadstrajectory_tree_data(CoreLoadsTrajectory& loads);
  static std::vector<std::vector<std::string>> get_loadsfilm_tree_data(CoreLoadsFilm& loads);
  static std::vector<std::vector<std::string>> get_loadsradiation_tree_data(CoreLoadsRadiation& loads);
  static std::vector<std::vector<std::string>> get_loadssurfacetraction_tree_data(CoreLoadsSurfaceTraction& loads);
  static std::vector<std::vector<std::string>> get_loadsbodyheatflux_tree_data(CoreLoadsBodyHeatflux& loads);
};

#endif
