#include "CoreTopologyAdjointStepExport.hpp"
#include "CoreTopologyAdjointBoundaries.hpp"
#include "CoreTopologyAdjointOutputs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "ScientificNumber.hpp"
#include "CubitMessage.hpp"
#include <cstddef>
#include <set>
#include <sstream>
#include <utility>

std::string CoreTopologyAdjointStepExport::get_export(const CoreTopologyAdjointStepExportData& data, int node_id, const std::array<double, 3>& load)
{
  std::set<std::pair<int, int>> boundaries;

  if (!CoreTopologyAdjointBoundaries::collect(data.displacements, data.hbcs, data.steps, data.mesh, boundaries))
  {
    return "";
  }

  if (data.steps.steps_data.empty())
  {
    PRINT_ERROR("No analysis step is available for the adjoint solve.\n");
    return "";
  }

  const auto& final_step = data.steps.steps_data.back();

  if (final_step.size() < 9)
  {
    PRINT_ERROR("Invalid final analysis step data.\n");
    return "";
  }

  const std::string step_definition = data.steps.get_step_export(final_step[0]);

  if (step_definition.empty())
  {
    PRINT_ERROR("Unable to export the final analysis step definition.\n");
    return "";
  }

  std::ostringstream output;
  output << "** Topology adjoint calculation\n";
  output << step_definition;

  if (step_definition.back() != '\n')
  {
    output << "\n";
  }

  if (!boundaries.empty())
  {
    output << "*BOUNDARY\n";
    CoreMeshQuery mesh_query(data.mesh);

    for (const auto& boundary : boundaries)
    {
      const std::string nodeset_name = mesh_query.get_nodeset_name(boundary.first);

      if (nodeset_name.empty())
      {
        PRINT_ERROR("Missing adjoint boundary nodeset name.\n");
        return "";
      }

      output << nodeset_name << "," << boundary.second << "," << boundary.second << ",0.000000\n";
    }
  }

  output << "*CLOAD\n";

  for (std::size_t axis = 0; axis < load.size(); ++axis)
  {
    if (load[axis] != 0.0)
    {
      output << node_id << "," << axis + 1 << "," << ScientificNumber::format(load[axis], 6) << "\n";
    }
  }

  if (!data.fieldoutputs)
  {
    PRINT_ERROR("Field output core is not initialized.\n");
    return "";
  }

  if (!data.historyoutputs)
  {
    PRINT_ERROR("History output core is not initialized.\n");
    return "";
  }

  if (!CoreTopologyAdjointOutputs::append_history(data.steps, *data.historyoutputs, final_step[7], output))
  {
    return "";
  }

  if (!CoreTopologyAdjointOutputs::append_field(data.steps, *data.fieldoutputs, final_step[8], output))
  {
    return "";
  }

  output << "*END STEP\n";
  return output.str();
}
