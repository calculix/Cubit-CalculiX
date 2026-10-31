#ifndef COREBCREMOVAL_HPP
#define COREBCREMOVAL_HPP

class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreSteps;
class CoreHBCs;
class CoreInitialConditions;

class CoreBCRemoval
{
public:
  static bool delete_bc(CoreBCsDisplacements& bcs, CoreSteps& steps, CoreHBCs& hbcs, CoreInitialConditions& conditions, int bc_id);
  static bool delete_bc(CoreBCsTemperatures& bcs, CoreSteps& steps, CoreHBCs& hbcs, CoreInitialConditions& conditions, int bc_id);
};

#endif
