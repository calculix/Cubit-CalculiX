#include "CoreTopologyDisplacementResults.hpp"
#include "CoreTopology.hpp"
#include "CoreTopologyQuery.hpp"
#include "CoreTopologyResults.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/results/CoreResults.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <QString>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <set>
#include <string>

bool CoreTopologyDisplacementResults::read_final(CoreResults& results, MeshExportInterface& mesh, int job_id, int nodeset_id, int component, std::map<int, std::array<double, 3>>& displacements)
{
  displacements.clear();

  if (nodeset_id <= 0 || component < 0 || component > 3)
  {
    PRINT_ERROR("Invalid topology displacement request.\n");
    return false;
  }

  const int dat_index = results.get_dat_data_id_from_job_id(job_id);

  if (dat_index < 0)
  {
    PRINT_ERROR("Missing DAT results for topology displacement measurement.\n");
    return false;
  }

  auto& dat = results.dat_data[dat_index];
  const auto nodes = CubitInterface::get_nodeset_nodes_inclusive(nodeset_id);
  const std::set<int> required(nodes.begin(), nodes.end());
  const int final_step = dat.current_step;
  const int final_increment = dat.current_increment;

  if (required.empty() || final_step <= 0 || final_increment <= 0)
  {
    PRINT_ERROR("Missing displacement nodes or final solver increment.\n");
    return false;
  }

  const QString expected_set = QString::fromStdString(CoreMeshQuery(mesh).get_nodeset_name(nodeset_id));
  std::map<int, std::array<double, 3>> measured;

  for (const auto& block : dat.result_blocks)
  {
    if (block.size() < 7)
    {
      PRINT_ERROR("Invalid DAT result block.\n");
      return false;
    }

    if (block[5] != final_step || block[6] != final_increment)
    {
      continue;
    }

    const int type = block[2];
    const int set = block[3];
    const int data = block[4];

    if (type < 0 || static_cast<std::size_t>(type) >= dat.result_block_type.size() || set < 0 || static_cast<std::size_t>(set) >= dat.result_block_set.size())
    {
      PRINT_ERROR("Invalid DAT displacement block reference.\n");
      return false;
    }

    if (dat.erase_whitespace(dat.result_block_type[type]) != "displacements")
    {
      continue;
    }

    if (QString::compare(QString::fromStdString(dat.result_block_set[set]), expected_set, Qt::CaseInsensitive) != 0)
    {
      continue;
    }

    if (data < 0 || static_cast<std::size_t>(data) >= dat.result_block_components.size() || static_cast<std::size_t>(data) >= dat.result_block_data.size() || static_cast<std::size_t>(data) >= dat.result_block_c1_data.size())
    {
      PRINT_ERROR("Invalid DAT displacement data reference.\n");
      return false;
    }

    const auto& components = dat.result_block_components[data];
    std::array<int, 3> columns{};
    const std::array<std::string, 3> names = {"vx", "vy", "vz"};

    for (std::size_t axis = 0; axis < names.size(); ++axis)
    {
      const auto found = std::find(components.begin(), components.end(), names[axis]);

      if (found == components.end())
      {
        PRINT_ERROR("Missing displacement component %s.\n", names[axis].c_str());
        return false;
      }

      columns[axis] = static_cast<int>(found - components.begin());
    }

    for (const auto& record : dat.result_block_c1_data[data])
    {
      if (record.size() < 3)
      {
        PRINT_ERROR("Invalid DAT displacement record.\n");
        return false;
      }

      if (record[2] != 1 || required.count(record[0]) == 0)
      {
        continue;
      }

      const int row = record[1];

      if (row < 0 || static_cast<std::size_t>(row) >= dat.result_block_data[data].size())
      {
        PRINT_ERROR("Invalid DAT displacement row.\n");
        return false;
      }

      const auto& values = dat.result_block_data[data][row];
      std::array<double, 3> displacement{};

      for (std::size_t axis = 0; axis < columns.size(); ++axis)
      {
        if (static_cast<std::size_t>(columns[axis]) >= values.size() || !std::isfinite(values[columns[axis]]))
        {
          PRINT_ERROR("Missing or invalid displacement for node %d.\n", record[0]);
          return false;
        }

        displacement[axis] = values[columns[axis]];
      }

      const double measured_value = component == 0 ? std::hypot(displacement[0], displacement[1], displacement[2]) : std::abs(displacement[component - 1]);

      if (!std::isfinite(measured_value) || !measured.emplace(record[0], displacement).second)
      {
        PRINT_ERROR("Invalid or duplicate displacement for node %d.\n", record[0]);
        return false;
      }
    }
  }

  if (measured.size() != required.size())
  {
    PRINT_ERROR("Incomplete nodal displacements in the final solver step/increment.\n");
    return false;
  }

  displacements.swap(measured);
  return true;
}

bool CoreTopologyDisplacementResults::measure_iteration(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, MeshExportInterface& mesh, int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement)
{
  value = 0.0;
  node_id = -1;

  if (critical_displacement)
  {
    critical_displacement->fill(0.0);
  }

  const auto entry = topology.topology_cases.find(topology_id);

  if (entry == topology.topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto& configuration = entry->second;
  const int nodeset_id = configuration.displacement_nodeset;
  const int component = configuration.displacement_component;
  const int job_id = CoreTopologyQuery::get_iteration_job_id(topology, jobs, topology_id, iteration_id);

  if (nodeset_id <= 0 || component < 0 || component > 3 || job_id <= 0 || jobs.get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Invalid topology displacement request.\n");
    return false;
  }

  std::map<int, std::array<double, 3>> measured;

  if (!read_final(results, mesh, job_id, nodeset_id, component, measured))
  {
    return false;
  }

  return CoreTopologyResults::select_maximum_displacement(measured, component, value, node_id, critical_displacement);
}
