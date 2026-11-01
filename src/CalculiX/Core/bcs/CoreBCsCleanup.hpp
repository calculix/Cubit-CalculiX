#ifndef COREBCSCLEANUP_HPP
#define COREBCSCLEANUP_HPP

#include <string>

class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreAmplitudes;
class CoreSteps;
class CoreHBCs;
class CoreInitialConditions;
class MeshExportInterface;

struct CoreBCsCleanupData
{
  CoreAmplitudes& amplitudes;
  CoreSteps& steps;
  CoreHBCs& hbcs;
  CoreInitialConditions& conditions;
  MeshExportInterface& mesh;
};

class CoreBCsCleanup
{
public:
  static bool cleanup(CoreBCsDisplacements& bcs, const CoreBCsCleanupData& data, std::string& log);
  static bool cleanup(CoreBCsTemperatures& bcs, const CoreBCsCleanupData& data, std::string& log);

  static bool cleanup(CoreHBCs& hbcs, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, std::string& log);
};

#endif
