#include "Core/bcs/CoreBCsCleanup.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "Core/bcs/CoreBCRemoval.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"

#include <cstddef>

namespace
{
template<typename BCType>
bool cleanup_bcs(BCType& bcs, const CoreBCsCleanupData& data, const std::string& label, std::string& log)
{
  CoreMeshQuery query(data.mesh);
  bool changed = false;

  for (std::size_t index = bcs.bcs_data.size(); index > 0; index--)
  {
    auto& bc = bcs.bcs_data[index - 1];
    const int bc_id = bc[0];

    if (bc[2] != -1)
    {
      if (!data.amplitudes.check_amplitude_exists(bc[2]))
      {
        log.append("Amplitude ID " + std::to_string(bc[2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from BC " + label + " ID " + std::to_string(bc_id) + " will be deleted.\n");
        bc[2] = -1;
        changed = true;
      }
    }

    if (!query.check_nodeset_exists(bc[4]))
    {
      log.append("Nodeset ID " + std::to_string(bc[4]) + " doesn't exist.\n");
      log.append(label + " ID " + std::to_string(bc_id) + " will be deleted.\n");

      CoreBCRemoval::delete_bc(bcs, data.steps, data.hbcs, data.conditions, bc_id);
      changed = true;
    }
  }

  return changed;
}

template<typename BCType>
bool cleanup_hbc_references(CoreHBCs& hbcs, BCType& bcs, int bc_type, const std::string& label, std::string& log)
{
  const auto indices = hbcs.get_bc_data_ids_from_bcs_id(0);
  bool changed = false;

  for (std::size_t index = indices.size(); index > 0; index--)
  {
    const auto& reference = hbcs.bcs_data[indices[index - 1]];

    if (reference[1] == bc_type)
    {
      const int bc_id = reference[2];

      if (bcs.get_bcs_data_id_from_bc_id(bc_id) == -1)
      {
        log.append("BC " + label + " ID " + std::to_string(bc_id) + " doesn't exist.\n");
        log.append("BC " + label + " Reference from HBC will be deleted.\n");
        hbcs.remove_bcs(0, bc_type, {bc_id});
        changed = true;
      }
    }
  }

  return changed;
}
}

bool CoreBCsCleanup::cleanup(CoreBCsDisplacements& bcs, const CoreBCsCleanupData& data, std::string& log)
{
  return cleanup_bcs(bcs, data, "Displacement", log);
}

bool CoreBCsCleanup::cleanup(CoreBCsTemperatures& bcs, const CoreBCsCleanupData& data, std::string& log)
{
  return cleanup_bcs(bcs, data, "Temperature", log);
}

bool CoreBCsCleanup::cleanup(CoreHBCs& hbcs, CoreBCsDisplacements& displacements, CoreBCsTemperatures& temperatures, std::string& log)
{
  const bool displacement_changed = cleanup_hbc_references(hbcs, displacements, 1, "Displacement", log);
  const bool temperature_changed = cleanup_hbc_references(hbcs, temperatures, 2, "Temperature", log);
  return displacement_changed || temperature_changed;
}
