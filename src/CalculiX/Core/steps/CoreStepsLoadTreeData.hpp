#ifndef CORESTEPSLOADTREEDATA_HPP
#define CORESTEPSLOADTREEDATA_HPP

#include <string>
#include <vector>

class CoreSteps;
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

class CoreStepsLoadTreeData
{
public:
  static std::vector<std::vector<std::string>> get_steps_loadsforces_tree_data(
    CoreSteps& steps, CoreLoadsForces& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadspressures_tree_data(
    CoreSteps& steps, CoreLoadsPressures& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadsheatfluxes_tree_data(
    CoreSteps& steps, CoreLoadsHeatfluxes& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadsgravity_tree_data(
    CoreSteps& steps, CoreLoadsGravity& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadscentrifugal_tree_data(
    CoreSteps& steps, CoreLoadsCentrifugal& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadstrajectory_tree_data(
    CoreSteps& steps, CoreLoadsTrajectory& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadsfilm_tree_data(
    CoreSteps& steps, CoreLoadsFilm& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadsradiation_tree_data(
    CoreSteps& steps, CoreLoadsRadiation& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadssurfacetraction_tree_data(
    CoreSteps& steps, CoreLoadsSurfaceTraction& loads, int step_id);

  static std::vector<std::vector<std::string>> get_steps_loadsbodyheatflux_tree_data(
    CoreSteps& steps, CoreLoadsBodyHeatflux& loads, int step_id);
};

#endif
