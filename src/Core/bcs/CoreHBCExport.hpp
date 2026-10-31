#ifndef COREHBCEXPORT_HPP
#define COREHBCEXPORT_HPP

#include <string>

class CoreHBCs;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreCustomLines;
class MeshExportInterface;

class CoreHBCExport
{
public:
  static std::string get_export(
    CoreHBCs& hbcs, CoreBCsDisplacements& displacements,
    CoreBCsTemperatures& temperatures, CoreCustomLines& customlines,
    MeshExportInterface& mesh);
};

#endif
