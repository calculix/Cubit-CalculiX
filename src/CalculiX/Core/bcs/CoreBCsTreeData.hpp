#ifndef COREBCSTREEDATA_HPP
#define COREBCSTREEDATA_HPP

#include <string>
#include <vector>

class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreHBCs;

class CoreBCsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_bcsdisplacements_tree_data(CoreBCsDisplacements& bcs);
  static std::vector<std::vector<std::string>> get_bcstemperatures_tree_data(CoreBCsTemperatures& bcs);

  static std::vector<std::vector<std::string>> get_hbcsdisplacements_tree_data(
    CoreHBCs& hbcs, CoreBCsDisplacements& bcs);

  static std::vector<std::vector<std::string>> get_hbcstemperatures_tree_data(
    CoreHBCs& hbcs, CoreBCsTemperatures& bcs);
};

#endif
