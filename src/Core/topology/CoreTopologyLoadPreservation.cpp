#include "CoreTopologyLoadPreservation.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "CubitMessage.hpp"
#include <algorithm>

bool CoreTopologyLoadPreservation::append_set_loads(const std::vector<std::vector<int>>& rows, const std::set<int>& load_ids, int set_type, const char* load_name, std::map<int, std::set<int>>& sets)
{
  for (int load_id : load_ids)
  {
    const auto row = std::find_if(rows.begin(), rows.end(), [&](const std::vector<int>& data)
    {
      return !data.empty() && data[0] == load_id;
    });

    if (row == rows.end() || row->size() < 5 || (*row)[4] <= 0)
    {
      PRINT_ERROR("Missing or invalid %s load %d.\n", load_name, load_id);
      return false;
    }

    sets[set_type].insert((*row)[4]);
  }

  return true;
}

bool CoreTopologyLoadPreservation::append_body_heatflux(CoreLoadsBodyHeatflux& loads, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets, std::set<int>& elements)
{
  for (int load_id : load_ids)
  {
    const int load_index = loads.get_loads_data_id_from_load_id(load_id);

    if (load_index < 0)
    {
      PRINT_ERROR("Body heat flux load %d does not exist.\n", load_id);
      return false;
    }

    const auto& load = loads.loads_data[load_index];

    if (load.size() < 6 || (load[5] != 1 && load[5] != 2))
    {
      PRINT_ERROR("Invalid body heat flux selection.\n");
      return false;
    }

    const int index = loads.get_element_data_id_from_element_id(load[4]);

    if (index < 0 || loads.element_data[index].size() < 2)
    {
      PRINT_ERROR("Missing body heat flux selection data.\n");
      return false;
    }

    const auto& selection = loads.element_data[index];

    // Entry 0 is the selection record ID.
    for (std::size_t i = 1; i < selection.size(); ++i)
    {
      const int id = selection[i];

      if (id <= 0)
      {
        PRINT_ERROR("Invalid body heat flux target ID %d.\n", id);
        return false;
      }

      if (load[5] == 1)
      {
        sets[1].insert(id);
      }
      else
      {
        elements.insert(id);
      }
    }
  }

  return true;
}

namespace
{
template <typename Loads>
bool append_mesh_loads(Loads& loads, MeshExportInterface& mesh, const std::set<int>& load_ids, int set_type, const char* load_name, std::map<int, std::set<int>>& sets)
{
  CoreMeshQuery mesh_query(mesh);

  for (int load_id : load_ids)
  {
    const int index = loads.get_loads_data_id_from_load_id(load_id);

    if (index < 0 || loads.loads_data[index].size() < 7)
    {
      PRINT_ERROR("Missing or invalid %s load %d.\n", load_name, load_id);
      return false;
    }

    const int set_id = loads.loads_data[index][4];
    const bool exists = set_type == 2 ? mesh_query.check_nodeset_exists(set_id) : mesh_query.check_sideset_exists(set_id);

    if (!exists)
    {
      const char* entity = set_type == 2 ? "Nodeset" : "Sideset";
      PRINT_ERROR("%s %d referenced by %s %d does not exist.\n", entity, set_id, load_name, load_id);
      return false;
    }

    sets[set_type].insert(set_id);
  }

  return true;
}
}

bool CoreTopologyLoadPreservation::append_loads(CoreLoadsForces& loads, MeshExportInterface& mesh, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets)
{
  return append_mesh_loads(loads, mesh, load_ids, 2, "force", sets);
}

bool CoreTopologyLoadPreservation::append_loads(CoreLoadsPressures& loads, MeshExportInterface& mesh, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets)
{
  return append_mesh_loads(loads, mesh, load_ids, 3, "pressure", sets);
}

bool CoreTopologyLoadPreservation::append_loads(CoreLoadsHeatfluxes& loads, MeshExportInterface& mesh, const std::set<int>& load_ids, std::map<int, std::set<int>>& sets)
{
  return append_mesh_loads(loads, mesh, load_ids, 3, "heatflux", sets);
}
