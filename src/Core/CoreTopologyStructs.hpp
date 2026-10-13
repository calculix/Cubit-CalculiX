#ifndef CORETOPOLOGYSTRUCTS_HPP
#define CORETOPOLOGYSTRUCTS_HPP

#include <map>
#include <string>
#include <vector>
#include <set>

enum class TopologyMethod
{
  Unspecified,
  Density,
  LevelSet
};

enum class TopologyObjective
{
  Unspecified,
  Compliance,
  Mass,
  Displacement
};

struct TopologyIterationEvaluation
{
  TopologyObjective objective = TopologyObjective::Unspecified;
  double value = 0.0;

  // Global element ID -> derivative with respect to stiffness factor.
  // Preserved elements receive zero because they are fixed.
  std::map<int, double> stiffness_sensitivities;
  // Global element ID -> derivative with respect to solid fraction.
  // Empty when no density design belongs to the iteration.
  std::map<int, double> density_sensitivities;
};

struct TopologyVolumeEvaluation
{
  /*
  Total volume:       Σ V_e
  Solid volume:       Σ ρ_e V_e
  Preserved volume:   Σ V_e for preserved elements
  Volume fraction:    solid volume / total volume
  Minimum fraction:   preserved volume / total volume
  */
  bool available = false;
  double total_volume = 0.0;
  double solid_volume = 0.0;
  double preserved_volume = 0.0;
  double volume_fraction = 0.0;
  double minimum_volume_fraction = 0.0;
  // Includes preserved elements and the density floor of free elements.
  double minimum_design_volume_fraction = 0.0;

  // Derivative of volume fraction with respect to element density.
  // Preserved elements receive zero because they are fixed.
  std::map<int, double> density_sensitivities;
};

struct TopologyDensityDesign
{
  // Store interpolation settings with the iteration.
  double penalization = 0.0;
  double void_ratio = 0.0;
  double minimum_density = 0.0;

  // Global element ID -> solid fraction.
  std::map<int, double> element_densities;
  TopologyVolumeEvaluation volume;
};

struct TopologyCase
{
  std::string name;
  TopologyMethod method;
  TopologyObjective objective;

  // Set types follow the step add/remove approach:
  // 1 = block, 2 = nodeset, 3 = sideset.

  std::map<int, std::set<int>> added_preserved_sets;
  std::map<int, std::set<int>> removed_preserved_sets;

  // Global volume element ID -> preserved.
  std::map<int, bool> element_preserved;

  double target_volume_fraction;
  double void_stiffness_ratio;
  int maximum_iterations;
  double density_penalization;
  double minimum_density;

  std::map<int, int> iteration_jobs;
  int selected_iteration = -1;
  std::map<int, std::map<int, double>> iteration_element_energies; // Iteration ID -> global element ID -> internal energy.
  std::map<int, double> iteration_total_energies;
  std::map<int, TopologyIterationEvaluation> iteration_evaluations;
  std::map<int, TopologyDensityDesign> iteration_density_designs;
};

struct TopologyOptions
{
  bool has_name = false;
  std::string name;

  bool has_method = false;
  TopologyMethod method = TopologyMethod::Unspecified;

  bool has_objective = false;
  TopologyObjective objective = TopologyObjective::Unspecified;

  bool has_volume_fraction = false;
  double volume_fraction = 0.0;

  bool has_void_ratio = false;
  double void_ratio = 0.0;

  bool has_maximum_iterations = false;
  int maximum_iterations = 0;
};

#endif