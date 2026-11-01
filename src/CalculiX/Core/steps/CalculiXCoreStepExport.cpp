#include "Core/CalculiXCore.hpp"
#include "CoreStepExport.hpp"
#include "Core/topology/CoreTopologyAdjointStepExport.hpp"
#include "CubitMessage.hpp"

std::string CalculiXCore::get_step_export_data()
{
  if (is_topology_adjoint_export())
  {
    double objective_value = 0.0;
    int node_id = -1;
    std::array<double, 3> load{};

    if (!build_topology_adjoint_load(job_export_context.topology_id, job_export_context.iteration_id, objective_value, node_id, load))
    {
      return "";
    }

    if (objective_value == 0.0)
    {
      return "";
    }

    if (!bcsdisplacements || !hbcs || !steps || !me_iface)
    {
      PRINT_ERROR("Adjoint boundary interfaces are not initialized.\n");
      return "";
    }

    const CoreTopologyAdjointStepExportData data = {
      *steps,
      *bcsdisplacements,
      *hbcs,
      *me_iface,
      fieldoutputs,
      historyoutputs
    };

    return CoreTopologyAdjointStepExport::get_export(data, node_id, load);
  }

  const CoreStepExportData data = {
    *steps,
    *customlines,
    *bcsdisplacements,
    *bcstemperatures,
    *historyoutputs,
    *fieldoutputs,
    *loadsforces,
    *loadspressures,
    *loadsheatfluxes,
    *loadsgravity,
    *loadscentrifugal,
    *loadsfilm,
    *loadsradiation,
    *loadssurfacetraction,
    *loadsbodyheatflux,
    sideset_faces,
    *me_iface
  };

  return CoreStepExport::get_export(data);
}
