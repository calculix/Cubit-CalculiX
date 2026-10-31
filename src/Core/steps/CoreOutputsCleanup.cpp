#include "Core/steps/CoreOutputsCleanup.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"
#include "Core/interactions/CoreContactPairs.hpp"

#include <cstddef>
#include <vector>

namespace
{
template<typename OutputType>
bool cleanup_outputs(OutputType& outputs, CoreBlocks& blocks, CoreContactPairs& contacts, MeshExportInterface& mesh, const std::string& label, bool check_contacts, std::string& log)
{
  CoreMeshQuery query(mesh);
  bool changed = false;

  for (std::size_t index = outputs.outputs_data.size(); index > 0; index--)
  {
    const auto& output = outputs.outputs_data[index - 1];
    std::vector<std::vector<std::string>>* data = nullptr;
    int data_index = -1;
    std::string target_label;

    if (output[2] == 1)
    {
      data = &outputs.node_data;
      data_index = outputs.get_node_data_id_from_node_id(output[3]);
      target_label = "Nodeset";
    }
    else if (output[2] == 2)
    {
      data = &outputs.element_data;
      data_index = outputs.get_element_data_id_from_element_id(output[3]);
      target_label = "Block";
    }
    else if (output[2] == 3 && check_contacts)
    {
      data = &outputs.contact_data;
      data_index = outputs.get_contact_data_id_from_contact_id(output[3]);
      target_label = "Contact Pair";
    }

    if (!data)
    {
      continue;
    }

    if (data_index < 0)
    {
      const int output_id = output[0];
      log.append("Subtype data for " + label + " Output ID " + std::to_string(output_id) + " doesn't exist.\n");
      log.append(label + " Output ID " + std::to_string(output_id) + " will be deleted.\n");
      changed = outputs.delete_output(output_id) || changed;
      continue;
    }

    std::string& reference = (*data)[data_index][1];

    if (reference.empty())
    {
      continue;
    }

    const int target_id = std::stoi(reference);
    bool exists = false;

    if (output[2] == 1)
    {
      exists = query.check_nodeset_exists(target_id);
    }
    else if (output[2] == 2)
    {
      exists = blocks.check_block_exists(target_id);
    }
    else
    {
      exists = contacts.check_contactpair_exists(target_id);
    }

    if (!exists)
    {
      log.append(target_label + " ID " + reference + " doesn't exist.\n");
      log.append(target_label + " Reference from " + label + " Output ID " + std::to_string(output[0]) + " will be deleted.\n");
      reference.clear();
      changed = true;
    }
  }

  return changed;
}
}

bool CoreOutputsCleanup::cleanup(CoreHistoryOutputs& outputs, CoreBlocks& blocks, CoreContactPairs& contacts, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_outputs(outputs, blocks, contacts, mesh, "History", true, log);
}

bool CoreOutputsCleanup::cleanup(CoreFieldOutputs& outputs, CoreBlocks& blocks, CoreContactPairs& contacts, MeshExportInterface& mesh, std::string& log)
{
  return cleanup_outputs(outputs, blocks, contacts, mesh, "Field", false, log);
}
