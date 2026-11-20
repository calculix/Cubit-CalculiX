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

    if (constraint[1] < 2 || constraint[1] > 4)
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
