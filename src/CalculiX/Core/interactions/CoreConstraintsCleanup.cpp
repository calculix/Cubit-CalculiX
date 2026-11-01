#include "Core/interactions/CoreConstraintsCleanup.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CoreMeshQuery.hpp"

#include <cstddef>

bool CoreConstraintsCleanup::cleanup(CoreConstraints& constraints, CoreBlocks& blocks, MeshExportInterface& mesh, std::string& log)
{
  CoreMeshQuery query(mesh);
  bool changed = false;

  for (std::size_t index = constraints.constraints_data.size(); index > 0; index--)
  {
    const auto& constraint = constraints.constraints_data[index - 1];
    const int constraint_id = constraint[0];
    bool remove_constraint = false;

    const auto missing_reference = [&](const std::string& message)
    {
      log.append(message);
      log.append("Constraint ID " + std::to_string(constraint_id) + " will be deleted.\n");
      remove_constraint = true;
    };

    if (constraint[1] < 1 || constraint[1] > 4)
    {
      continue;
    }

    const auto* data = constraints.get_constraint_data(constraint_id);

    if (!data)
    {
      const char* labels[] = {"Rigid-body", "Tie", "Equation", "Equation-group"};
      missing_reference(std::string(labels[constraint[1] - 1]) + " data for Constraint ID " + std::to_string(constraint_id) + " doesn't exist.\n");
      changed = constraints.delete_constraint(constraint_id) || changed;
      continue;
    }

    if (constraint[1] == 1)
    {
      const auto& row = *data;

      if (row[1] == "1")
      {
        if (!query.check_nodeset_exists(std::stoi(row[2])))
        {
          missing_reference("Nodeset ID " + row[2] + " doesn't exist.\n");
        }
      }
      else if (row[1] == "2")
      {
        if (blocks.get_blocks_data_id_from_block_id(std::stoi(row[2])) == -1)
        {
          missing_reference("Block ID " + row[2] + " doesn't exist.\n");
        }
      }

      for (std::size_t column = 3; column <= 4; column++)
      {
        if (!CoreMeshQuery::check_vertex_exists(std::stoi(row[column])))
        {
          missing_reference("Vertex ID " + row[column] + " doesn't exist.\n");
        }
      }
    }

    if (constraint[1] == 2)
    {
      const auto& row = *data;

      if (!query.check_sideset_exists(std::stoi(row[2])))
      {
        missing_reference("Master: Sideset ID " + row[2] + " doesn't exist.\n");
      }

      if (!query.check_sideset_exists(std::stoi(row[3])))
      {
        missing_reference("Slave: Sideset ID " + row[3] + " doesn't exist.\n");
      }
    }

    if (remove_constraint)
    {
      constraints.delete_constraint(constraint_id);
      changed = true;
    }
  }

  return changed;
}
