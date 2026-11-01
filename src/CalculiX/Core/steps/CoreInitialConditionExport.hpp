#ifndef COREINITIALCONDITIONEXPORT_HPP
#define COREINITIALCONDITIONEXPORT_HPP

#include <string>

class CoreInitialConditions;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreCustomLines;
class CoreBlocks;
class MeshExportInterface;

class CoreInitialConditionExport
{
public:
  static std::string get_export(
    CoreInitialConditions& conditions, CoreBCsDisplacements& displacements,
    CoreBCsTemperatures& temperatures, CoreCustomLines& customlines,
    CoreBlocks& blocks, MeshExportInterface& mesh);
};

#endif
