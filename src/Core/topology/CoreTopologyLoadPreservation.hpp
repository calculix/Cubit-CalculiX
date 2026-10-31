#ifndef CORETOPOLOGYLOADPRESERVATION_HPP
#define CORETOPOLOGYLOADPRESERVATION_HPP

#include <map>
#include <set>
#include <vector>

class CoreLoadsBodyHeatflux;
class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class MeshExportInterface;

class CoreTopologyLoadPreservation
{
public:
  static bool append_set_loads(const std::vector<std::vector<int>>& rows, const std::set<int>& load_ids, int set_type, const char* load_name, std::map<int, std::set<int>>& sets);
  static bool append_body_heatflux(CoreLoadsBodyHeatflux& loads, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets, std::set<int>& elements);
  static bool append_loads(CoreLoadsForces& loads, MeshExportInterface& mesh, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets);
  static bool append_loads(CoreLoadsPressures& loads, MeshExportInterface& mesh, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets);
  static bool append_loads(CoreLoadsHeatfluxes& loads, MeshExportInterface& mesh, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets);
};

#endif
