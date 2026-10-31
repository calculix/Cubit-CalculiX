#ifndef CORESTEPSBCTREEDATA_HPP
#define CORESTEPSBCTREEDATA_HPP

#include <string>
#include <vector>

class CoreSteps;
class CoreBCsDisplacements;
class CoreBCsTemperatures;

class CoreStepsBCTreeData
{
public:
  static std::vector<std::vector<std::string>> get_steps_bcsdisplacements_tree_data(
    CoreSteps& steps, CoreBCsDisplacements& bcs, int step_id);

  static std::vector<std::vector<std::string>> get_steps_bcstemperatures_tree_data(
    CoreSteps& steps, CoreBCsTemperatures& bcs, int step_id);
};

#endif
