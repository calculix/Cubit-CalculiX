#ifndef COREINITIALCONDITIONSTREEDATA_HPP
#define COREINITIALCONDITIONSTREEDATA_HPP

#include <string>
#include <vector>

class CoreInitialConditions;
class CoreBCsDisplacements;
class CoreBCsTemperatures;

class CoreInitialConditionsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_initialconditions_tree_data(
    CoreInitialConditions& conditions,
    CoreBCsDisplacements& displacements,
    CoreBCsTemperatures& temperatures);
};

#endif
