#include "CoreTopologyInteractionPreservation.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <limits>
#include <sstream>
#include <string>

namespace
{
bool append_set_id(std::map<int, std::set<int>>& sets, int set_type, const std::string& text)
{
  int id = -1;
  std::istringstream input(text);

  if (!(input >> id) || id <= 0)
  {
    PRINT_ERROR("Invalid preserved set ID '%s'.\n", text.c_str());
    return false;
  }

  input >> std::ws;

  if (!input.eof())
  {
    PRINT_ERROR("Invalid preserved set ID '%s'.\n", text.c_str());
    return false;
  }

  sets[set_type].insert(id);
  return true;
}

bool append_node_id(std::set<int>& nodes, double value)
{
  if (!std::isfinite(value) ||
      value < 1.0 ||
      value > static_cast<double>(std::numeric_limits<int>::max()) ||
      std::floor(value) != value)
  {
    PRINT_ERROR("Invalid preserved node ID.\n");
    return false;
  }

  const int node_id = static_cast<int>(value);

  if (!CubitInterface::get_node_exists(node_id))
  {
    PRINT_ERROR("Preserved node %d does not exist.\n", node_id);
    return false;
  }

  nodes.insert(node_id);
  return true;
}
}

bool CoreTopologyInteractionPreservation::append_constraints(CoreConstraints& constraints, std::map<int, std::set<int>>& sets, std::set<int>& nodes)
{
  for (const auto& constraint : constraints.constraints_data)
  {
    if (constraint.size() < 3)
    {
      PRINT_ERROR("Invalid constraint data.\n");
      return false;
    }

    const int type = constraint[1];
    const int subtype_id = constraint[2];

    if (type == 1)
    {
      const int index = constraints.get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(subtype_id);

      if (index < 0 || constraints.rigidbody_constraint_data[index].size() < 3)
      {
        PRINT_ERROR("Missing rigid body constraint data.\n");
        return false;
      }

      const auto& rigid = constraints.rigidbody_constraint_data[index];

      // Rigid body types: 1 = nodeset, 2 = block.
      // Preservation types: 1 = block, 2 = nodeset.
      int set_type = -1;

      if (rigid[1] == "1")
      {
        set_type = 2;
      }
      else if (rigid[1] == "2")
      {
        set_type = 1;
      }
      else
      {
        PRINT_ERROR("Invalid rigid body selection type.\n");
        return false;
      }

      if (!append_set_id(sets, set_type, rigid[2]))
      {
        return false;
      }
    }
    else if (type == 2)
    {
      const int index = constraints.get_tie_constraint_data_id_from_tie_constraint_id(subtype_id);

      if (index < 0 || constraints.tie_constraint_data[index].size() < 4)
      {
        PRINT_ERROR("Missing tie constraint data.\n");
        return false;
      }

      const auto& tie = constraints.tie_constraint_data[index];

      if (!append_set_id(sets, 3, tie[2]) || !append_set_id(sets, 3, tie[3]))
      {
        return false;
      }
    }
    else if (type == 3)
    {
      const auto indices = constraints.get_equation_data_ids_from_equation_constraint_id(subtype_id);

      if (indices.empty())
      {
        PRINT_ERROR("Missing equation constraint data.\n");
        return false;
      }

      for (int index : indices)
      {
        const auto& equation = constraints.equation_data[index];

        if (equation.size() < 4)
        {
          PRINT_ERROR("Invalid equation constraint data.\n");
          return false;
        }

        if (!append_node_id(nodes, equation[1]))
        {
          return false;
        }
      }
    }
    else if (type == 4)
    {
      const auto indices = constraints.get_equation_group_data_ids_from_equation_group_constraint_id(subtype_id);

      if (indices.empty())
      {
        PRINT_ERROR("Missing equation group constraint data.\n");
        return false;
      }

      for (int index : indices)
      {
        const auto& equation = constraints.equation_group_data[index];

        if (equation.size() < 4)
        {
          PRINT_ERROR("Invalid equation group constraint data.\n");
          return false;
        }

        if (!append_node_id(nodes, equation[1]) || !append_node_id(nodes, equation[2]))
        {
          return false;
        }
      }
    }
  }

  return true;
}

bool CoreTopologyInteractionPreservation::append_contact_pairs(CoreContactPairs& contacts, std::map<int, std::set<int>>& sets)
{
  for (const auto& contact : contacts.contactpairs_data)
  {
    if (contact.size() < 6 || contact[3] <= 0 || contact[4] <= 0)
    {
      PRINT_ERROR("Invalid contact pair data.\n");
      return false;
    }

    sets[3].insert(contact[3]);
    sets[3].insert(contact[4]);

    const int index = contacts.get_adjust_contactpair_data_id_from_adjust_contactpair_id(contact[5]);

    if (index < 0 || contacts.adjust_contactpair_data[index].size() < 3)
    {
      PRINT_ERROR("Missing contact pair adjustment data.\n");
      return false;
    }

    const std::string& nodeset = contacts.adjust_contactpair_data[index][2];

    if (!nodeset.empty() && !append_set_id(sets, 2, nodeset))
    {
      return false;
    }
  }

  return true;
}
