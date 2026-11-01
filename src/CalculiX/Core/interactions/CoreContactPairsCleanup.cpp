#include "Core/interactions/CoreContactPairsCleanup.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"

#include <cstddef>

bool CoreContactPairsCleanup::cleanup(CoreContactPairs& contacts, CoreSurfaceInteractions& interactions, MeshExportInterface& mesh, std::string& log)
{
  CoreMeshQuery query(mesh);
  bool changed = false;

  for (std::size_t index = contacts.contactpairs_data.size(); index > 0; index--)
  {
    const auto& pair = contacts.contactpairs_data[index - 1];
    const int contact_id = pair[0];
    bool remove_contact = false;

    const auto missing_reference = [&](const std::string& message)
    {
      log.append(message);
      log.append("Contact Pair ID " + std::to_string(contact_id) + " will be deleted.\n");
      remove_contact = true;
    };

    if (!interactions.check_surfaceinteraction_exists(pair[1]))
    {
      missing_reference("Surfaceinteraction ID " + std::to_string(pair[1]) + " doesn't exist.\n");
    }

    const int adjust_index = contacts.get_adjust_contactpair_data_id_from_adjust_contactpair_id(pair[5]);

    if (adjust_index == -1)
    {
      missing_reference("Adjustment ID " + std::to_string(pair[5]) + " doesn't exist.\n");
    }
    else
    {
      const auto& adjustment = contacts.adjust_contactpair_data[adjust_index];

      if (!adjustment[2].empty())
      {
        if (!query.check_nodeset_exists(std::stoi(adjustment[2])))
        {
          missing_reference("Nodeset ID " + adjustment[2] + " doesn't exist.\n");
        }
      }
    }

    if (!query.check_sideset_exists(pair[3]))
    {
      missing_reference("Master: Sideset ID " + std::to_string(pair[3]) + " doesn't exist.\n");
    }

    if (!query.check_sideset_exists(pair[4]))
    {
      missing_reference("Slave: Sideset ID " + std::to_string(pair[4]) + " doesn't exist.\n");
    }

    if (remove_contact)
    {
      contacts.delete_contactpair(contact_id);
      changed = true;
    }
  }

  return changed;
}
