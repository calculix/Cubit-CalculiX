#include "CoreConstraintsEntities.hpp"
#include "CoreConstraints.hpp"

std::vector<std::vector<std::string>> CoreConstraintsEntities::get_entities(CoreConstraints& constraints, int constraint_id)
{
  const int index = constraints.get_constraints_data_id_from_constraint_id(constraint_id);

  if (index == -1)
  {
    return {};
  }

  const auto* subtype_data = constraints.get_constraint_data(constraint_id);

  if (!subtype_data)
  {
    return {};
  }

  const auto& constraint = constraints.constraints_data[index];
  const int type = constraint[1];
  const int subtype_id = constraint[2];

  if (type == 1)
  {
    const auto& data = *subtype_data;

    if (data[1] == "1")
    {
      return {{"nodeset", data[2]}, {"vertex", data[3]}};
    }

    if (data[1] == "2")
    {
      return {{"block", data[2]}, {"vertex", data[3]}};
    }

    return {};
  }

  if (type == 2)
  {
    const auto& data = *subtype_data;
    return {{"sideset", data[2]}, {"sideset", data[3]}};
  }

  if (type == 3)
  {
    std::vector<std::vector<std::string>> entities;
    const auto data_ids = constraints.get_equation_data_ids_from_equation_constraint_id(subtype_id);

    for (int data_id : data_ids)
    {
      entities.push_back({"node", std::to_string(static_cast<int>(constraints.equation_data[data_id][1]))});
    }

    return entities;
  }

  if (type == 4)
  {
    std::string node_ids;
    const auto data_ids = constraints.get_equation_group_data_ids_from_equation_group_constraint_id(subtype_id);

    for (int data_id : data_ids)
    {
      const auto& data = constraints.equation_group_data[data_id];
      node_ids += " " + std::to_string(static_cast<int>(data[1]));
      node_ids += " " + std::to_string(static_cast<int>(data[2]));
    }

    return {{"node", node_ids}};
  }

  return {};
}
