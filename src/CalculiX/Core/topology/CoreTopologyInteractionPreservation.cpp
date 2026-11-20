#include "CoreTopologyInteractionPreservation.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "CubitInterface.hpp"
#include "Core/topology/CoreTopologyConsoleMessages.hpp"
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
    TOPOLOGY_ERROR("Invalid preserved set ID '%s'.\n", text.c_str());
    return false;
  }

  input >> std::ws;

  if (!input.eof())
  {
    TOPOLOGY_ERROR("Invalid preserved set ID '%s'.\n", text.c_str());
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
    TOPOLOGY_ERROR("Invalid preserved node ID.\n");
    return false;
  }

  const int node_id = static_cast<int>(value);

  if (!CubitInterface::get_node_exists(node_id))
  {
    TOPOLOGY_ERROR("Preserved node %d does not exist.\n", node_id);
    return false;
  }

  nodes.insert(node_id);
  return true;
}
}

bool CoreTopologyInteractionPreservation::append_constraints(CoreConstraints& constraints, std::map<int, std::set<int>>& sets, std::set<int>& nodes)
{
  CalculiXConnectionsInterface connections;
  std::set<int> vertices;
  std::set<int> points;
  const auto preserve_endpoint = [&](const ConnectionEndpoint& endpoint)
  {
    if (endpoint.kind == ConnectionEndpointKind::Vertex)
    {
      vertices.insert(endpoint.id);
    }
    else
    {
      points.insert(endpoint.id);
    }
  };
  for (const auto& mass : connections.point_masses())
  {
    preserve_endpoint(mass.endpoint);
  }
  for (const auto& spring : connections.springs())
  {
    preserve_endpoint(spring.first);
    if (spring.second)
    {
      preserve_endpoint(*spring.second);
    }
  }
  for (const auto& point : connections.points())
  {
    if (point.geometry == "vertex" && points.count(point.id))
    {
      const int node = CubitInterface::get_vertex_node(point.entity);
      if (node > 0)
      {
        nodes.insert(node);
      }
    }
  }
  for (int vertex : vertices)
  {
    if (!append_node_id(nodes, CubitInterface::get_vertex_node(vertex)))
    {
      return false;
    }
  }
  for (const auto& attachment : connections.attachments())
  {
    if (attachment.geometry == "sideset")
    {
      sets[3].insert(attachment.entity);
    }
    else
    {
      const auto members = CubitInterface::parse_cubit_list("node", "in surface " + std::to_string(attachment.entity));
      nodes.insert(members.begin(), members.end());
    }
  }
  for (const auto& body : connections.bodies())
  {
    if (body.geometry == "block" || body.geometry == "nodeset")
    {
      sets[body.geometry == "block" ? 1 : 2].insert(body.entity);
    }
    else
    {
      const auto members = CubitInterface::parse_cubit_list("node", "in " + body.geometry + " " + std::to_string(body.entity));
      nodes.insert(members.begin(), members.end());
    }
  }
  for (const auto& constraint : constraints.constraints_data)
  {
    if (constraint.size() < 3)
    {
      TOPOLOGY_ERROR("Invalid constraint data.\n");
      return false;
    }

    const int type = constraint[1];
    const int subtype_id = constraint[2];

    if (type == 2)
    {
      const int index = constraints.get_tie_constraint_data_id_from_tie_constraint_id(subtype_id);

      if (index < 0 || constraints.tie_constraint_data[index].size() < 4)
      {
        TOPOLOGY_ERROR("Missing tie constraint data.\n");
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
        TOPOLOGY_ERROR("Missing equation constraint data.\n");
        return false;
      }

      for (int index : indices)
      {
        const auto& equation = constraints.equation_data[index];

        if (equation.size() < 4)
        {
          TOPOLOGY_ERROR("Invalid equation constraint data.\n");
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
        TOPOLOGY_ERROR("Missing equation group constraint data.\n");
        return false;
      }

      for (int index : indices)
      {
        const auto& equation = constraints.equation_group_data[index];

        if (equation.size() < 4)
        {
          TOPOLOGY_ERROR("Invalid equation group constraint data.\n");
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
      TOPOLOGY_ERROR("Invalid contact pair data.\n");
      return false;
    }

    sets[3].insert(contact[3]);
    sets[3].insert(contact[4]);

    const int index = contacts.get_adjust_contactpair_data_id_from_adjust_contactpair_id(contact[5]);

    if (index < 0 || contacts.adjust_contactpair_data[index].size() < 3)
    {
      TOPOLOGY_ERROR("Missing contact pair adjustment data.\n");
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
