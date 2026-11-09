#include "Core/CalculiXCore.hpp"
#include "CoreTopologyModelValidation.hpp"
#include "CoreTopologyExportValidation.hpp"
#include "CubitMessage.hpp"

bool CalculiXCore::validate_topology_compliance_model(bool require_zero_displacements)
{
  if (!steps || !sections || !me_iface || !bcsdisplacements || !contactpairs || !initialconditions || !hbcs)
  {
    PRINT_ERROR("Compliance validation interfaces are not initialized.\n");
    return false;
  }

  const CoreTopologyModelValidationData data = {
    *steps,
    *sections,
    *me_iface,
    *bcsdisplacements,
    *contactpairs,
    *initialconditions,
    *hbcs,
    cb
  };

  return CoreTopologyModelValidation::validate_model(data, require_zero_displacements);
}

bool CalculiXCore::check_topology_displacement_selection(int topology_id, int iteration_id)
{
  if (!topology)
  {
    PRINT_ERROR("Invalid topology displacement selection request.\n");
    return false;
  }

  return CoreTopologyModelValidation::validate_displacement_selection(*topology, topology_id, iteration_id);
}

bool CalculiXCore::check_topology_export(int topology_id, int iteration_id)
{
  const auto validate_model = [this]()
  {
    return validate_topology_compliance_model(false);
  };

  return CoreTopologyExportValidation::check(job_export_context, customlines, topology_id, iteration_id, validate_model);
}

bool CalculiXCore::is_topology_level_set_run_enabled()
{
  return CoreTopologyModelValidation::level_set_run_enabled();
}
