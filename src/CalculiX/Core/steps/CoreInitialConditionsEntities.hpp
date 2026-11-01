#ifndef COREINITIALCONDITIONSENTITIES_HPP
#define COREINITIALCONDITIONSENTITIES_HPP

#include <string>
#include <vector>

class CoreInitialConditions;
class CoreBCsDisplacements;
class CoreBCsTemperatures;

class CoreInitialConditionsEntities
{
public:
  static std::vector<std::vector<std::string>> get_entities(
    CoreInitialConditions& conditions, CoreBCsDisplacements& displacements,
    CoreBCsTemperatures& temperatures, int initialcondition_id);
};

#endif
