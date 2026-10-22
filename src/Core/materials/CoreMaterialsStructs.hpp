#ifndef COREMATERIALSSTRUCTS_HPP
#define COREMATERIALSSTRUCTS_HPP

#include <map>
#include <vector>

struct TopologyElasticMaterial
{
  double youngs_modulus;
  double poissons_ratio;
};

struct TopologyMaterialGroup
{
  int id = 0;
  int base_material_id = -1;
  double stiffness_factor = 1.0;
};

struct TopologyMaterialGrouping
{
  int material_cap = 1024;
  double maximum_relative_factor_error = 0.0;
  std::vector<TopologyMaterialGroup> groups;
  std::map<int, int> element_group; // Element ID -> index in groups.
};

struct TopologyMaterialData
{
  // Original elastic properties, captured when the run starts.
  std::map<int, TopologyElasticMaterial> base_materials;

  // Global element ID -> original material ID.
  std::map<int, int> element_base_material;

  // Present only for grouped iterations; absent for historical legacy exports.
  std::map<int, int> iteration_material_caps;

  // Transient cache rebuilt exactly from stored applied factors after loading.
  std::map<int, TopologyMaterialGrouping> iteration_material_groups;

  // Iteration ID -> global element ID -> stiffness multiplier actually solved.
  std::map<int, std::map<int, double>> iteration_factors;
};

#endif // COREMATERIALSSTRUCTS_HPP