#ifndef CORETOPOLOGYADJOINTSTEPEXPORT_HPP
#define CORETOPOLOGYADJOINTSTEPEXPORT_HPP

#include <array>
#include <string>

class CoreSteps;
class CoreBCsDisplacements;
class CoreHBCs;
class MeshExportInterface;
class CoreFieldOutputs;
class CoreHistoryOutputs;

struct CoreTopologyAdjointStepExportData
{
  CoreSteps& steps;
  CoreBCsDisplacements& displacements;
  const CoreHBCs& hbcs;
  MeshExportInterface& mesh;
  CoreFieldOutputs* fieldoutputs;
  CoreHistoryOutputs* historyoutputs;
};

class CoreTopologyAdjointStepExport
{
public:
  static std::string get_export(const CoreTopologyAdjointStepExportData& data, int node_id, const std::array<double, 3>& load);
};

#endif
