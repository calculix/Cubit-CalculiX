#include "CoreCubitContactImport.hpp"
#include "CoreConstraints.hpp"
#include "CoreContactPairs.hpp"
#include "CoreSurfaceInteractions.hpp"
#include "CubitInterface.hpp"
#include "MeshExportInterface.hpp"

#include <cstddef>
#include <vector>

namespace
{
  bool get_contact_sidesets(MeshExportInterface& mesh, int contact_id, int& master_id, int& slave_id)
  {
    BCContactHandle contact;
    mesh.get_bc_contact_handle(contact_id, contact);

    BCEntityHandle master_region;
    BCEntityHandle slave_region;
    mesh.get_contact_regions(contact, master_region, slave_region);

    SidesetHandle master;
    SidesetHandle slave;

    if (!mesh.get_bc_sideset(master_region, master))
    {
      return false;
    }

    if (!mesh.get_bc_sideset(slave_region, slave))
    {
      return false;
    }

    master_id = mesh.id_from_handle(master);
    slave_id = mesh.id_from_handle(slave);
    return true;
  }
}

bool CoreCubitContactImport::create_ties(CoreConstraints& constraints, MeshExportInterface& mesh, const std::string& name, const std::string& position_tolerance)
{
  const auto contact_ids = CubitInterface::get_bc_id_list(CI_BCTYPE_CONTACT_PAIR);

  for (std::size_t i = 0; i < contact_ids.size(); ++i)
  {
    int master_id;
    int slave_id;

    if (!get_contact_sidesets(mesh, contact_ids[i], master_id, slave_id))
    {
      return false;
    }

    const std::vector<std::string> options = {
      name + std::to_string(i),
      std::to_string(master_id),
      std::to_string(slave_id),
      position_tolerance
    };

    if (!constraints.create_constraint("TIE", options, {}))
    {
      return false;
    }
  }

  return true;
}

bool CoreCubitContactImport::create_pairs(CoreContactPairs& pairs, CoreSurfaceInteractions& interactions, MeshExportInterface& mesh, int surfaceinteraction_id, const std::string& contactpairtype, const std::string& adjust, const std::string& adjust_nodeset)
{
  if (!interactions.check_surfaceinteraction_exists(surfaceinteraction_id))
  {
    return false;
  }

  const auto contact_ids = CubitInterface::get_bc_id_list(CI_BCTYPE_CONTACT_PAIR);

  for (int contact_id : contact_ids)
  {
    int master_id;
    int slave_id;

    if (!get_contact_sidesets(mesh, contact_id, master_id, slave_id))
    {
      return false;
    }

    const std::vector<std::string> options = {
      std::to_string(surfaceinteraction_id),
      contactpairtype,
      std::to_string(master_id),
      std::to_string(slave_id),
      adjust,
      adjust_nodeset
    };

    if (!pairs.create_contactpair(options))
    {
      return false;
    }
  }

  return true;
}
