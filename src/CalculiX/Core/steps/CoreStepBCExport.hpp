#ifndef CORESTEPBCEXPORT_HPP
#define CORESTEPBCEXPORT_HPP

#include <string>
#include <vector>

class CoreSteps;
class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreCustomLines;
class MeshExportInterface;

class CoreStepBCExport
{
public:
  static bool append(CoreSteps& steps, CoreBCsDisplacements& bcs, CoreCustomLines& customlines, MeshExportInterface& mesh, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
  static bool append(CoreSteps& steps, CoreBCsTemperatures& bcs, CoreCustomLines& customlines, const std::vector<int>& assignment_ids, std::vector<std::string>& lines);
};

#endif
