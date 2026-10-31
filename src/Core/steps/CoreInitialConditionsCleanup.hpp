#ifndef COREINITIALCONDITIONSCLEANUP_HPP
#define COREINITIALCONDITIONSCLEANUP_HPP

#include <string>

class CoreInitialConditions;
class CoreBCsDisplacements;
class CoreBCsTemperatures;

class CoreInitialConditionsCleanup
{
public:
  static bool cleanup(CoreInitialConditions& conditions, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, std::string& log);
};

#endif
