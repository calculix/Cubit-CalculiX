#ifndef CORETOPOLOGYBCPRESERVATION_HPP
#define CORETOPOLOGYBCPRESERVATION_HPP

#include <map>
#include <set>

class CoreBCsDisplacements;
class CoreBCsTemperatures;
class MeshExportInterface;

class CoreTopologyBCPreservation
{
public:
  static bool append_bcs(CoreBCsDisplacements& bcs, MeshExportInterface& mesh, const std::set<int>& bc_ids, std::map<int, std::set<int>>& sets);
  static bool append_bcs(CoreBCsTemperatures& bcs, MeshExportInterface& mesh, const std::set<int>& bc_ids, std::map<int, std::set<int>>& sets);
};

#endif
