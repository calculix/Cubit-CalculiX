#include "CoreInteractionsTreeData.hpp"
#include "CoreConstraints.hpp"
#include "CoreSurfaceInteractions.hpp"
#include "CoreContactPairs.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::vector<std::string>> CoreInteractionsTreeData::get_constraints_tree_data(CoreConstraints& constraints)
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& constraint : constraints.constraints_data)
  {
    const auto* data = constraints.get_constraint_data(constraint[0]);

    if (!data)
    {
      PRINT_ERROR("Constraint data missing or invalid for Constraint ID %d.\n", constraint[0]);
      continue;
    }

    std::string name;

    if (constraint[1] == 1)
    {
      std::string entity;

      if ((*data)[1] == "1")
      {
        entity = "Nodeset ";
      }
      else if ((*data)[1] == "2")
      {
        entity = "Block ";
      }

      name = "Rigid Body (" + entity + (*data)[2] + "| v_ref " + (*data)[3] + "| v_rot " + (*data)[4] + ")";
    }
    else if (constraint[1] == 2)
    {
      name = "TIE (" + (*data)[1] + ")";
    }
    else if (constraint[1] == 3)
    {
      name = "EQUATION (" + (*data)[1] + ")";
    }
    else if (constraint[1] == 4)
    {
      name = "EQUATION GROUP (" + (*data)[1] + ")";
    }

    tree_data.push_back({std::to_string(constraint[0]), name});
  }

  return tree_data;
}

std::vector<std::vector<std::string>> CoreInteractionsTreeData::get_surfaceinteractions_tree_data(CoreSurfaceInteractions& interactions)
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& interaction : interactions.surfaceinteractions_data)
  {
    const auto* name = interactions.find_surfaceinteraction_name(interaction[0]);

    if (!name)
    {
      PRINT_ERROR("Name data missing for Surface Interaction ID %d.\n", interaction[0]);
      continue;
    }

    tree_data.push_back({std::to_string(interaction[0]), *name});
  }

  return tree_data;
}

std::vector<std::vector<std::string>> CoreInteractionsTreeData::get_contactpairs_tree_data(CoreContactPairs& pairs)
{
  CalculiXMeshInterface mesh;
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& pair : pairs.contactpairs_data)
  {
    std::string name = "Master: ";
    name += mesh.get_sideset_name(pair[3]);
    name += " | Slave: ";
    name += mesh.get_sideset_name(pair[4]);

    tree_data.push_back({std::to_string(pair[0]), name});
  }

  return tree_data;
}
