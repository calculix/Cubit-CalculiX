#include "CoreTopologyBCPreservation.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitMessage.hpp"
#include <cstddef>

namespace
{
template <typename BCs>
bool append_nodesets(BCs& bcs, MeshExportInterface& mesh, const std::set<int>& bc_ids, std::size_t minimum_columns, const char* bc_name, std::map<int, std::set<int>>& sets)
{
  CoreMeshQuery mesh_query(mesh);

  for (int bc_id : bc_ids)
  {
    const int index = bcs.get_bcs_data_id_from_bc_id(bc_id);

    if (index < 0 || bcs.bcs_data[index].size() < minimum_columns)
    {
      PRINT_ERROR("Missing %s BC %d.\n", bc_name, bc_id);
      return false;
    }

    const int nodeset_id = bcs.bcs_data[index][4];

    if (!mesh_query.check_nodeset_exists(nodeset_id))
    {
      PRINT_ERROR("Nodeset %d referenced by %s BC %d does not exist.\n", nodeset_id, bc_name, bc_id);
      return false;
    }

    sets[2].insert(nodeset_id);
  }

  return true;
}
}

bool CoreTopologyBCPreservation::append_bcs(CoreBCsDisplacements& bcs, MeshExportInterface& mesh, const std::set<int>& bc_ids, std::map<int, std::set<int>>& sets)
{
  return append_nodesets(bcs, mesh, bc_ids, 7, "displacement", sets);
}

bool CoreTopologyBCPreservation::append_bcs(CoreBCsTemperatures& bcs, MeshExportInterface& mesh, const std::set<int>& bc_ids, std::map<int, std::set<int>>& sets)
{
  return append_nodesets(bcs, mesh, bc_ids, 8, "temperature", sets);
}
