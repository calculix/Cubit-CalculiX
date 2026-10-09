#ifndef COREMATERIALSSTRUCTS_HPP
#define COREMATERIALSSTRUCTS_HPP

#include <map>

struct TopologyElasticMaterial
{
  double youngs_modulus;
  double poissons_ratio;
};

struct TopologyMaterialData
{
  // Original elastic properties, captured when the run starts.
  std::map<int, TopologyElasticMaterial> base_materials;

  // Global element ID -> original material ID.
  std::map<int, int> element_base_material;

  // Iteration ID -> global element ID -> stiffness multiplier.
  std::map<int, std::map<int, double>> iteration_factors;
};

#endif // COREMATERIALSSTRUCTS_HPP