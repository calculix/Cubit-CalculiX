#include "CoreDrawConstraintData.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "CubitInterface.hpp"

#include <array>

std::vector<std::vector<double>> CoreDrawConstraintData::get_draw_data_for_equation(CoreConstraints& constraints, int id)
{
  std::vector<std::vector<double>> draw_data;
  int constraint_data_id = constraints.get_constraints_data_id_from_constraint_id(id);

  if (constraint_data_id == -1 || constraints.constraints_data[constraint_data_id][1] != 3)
  {
    return draw_data;
  }

  int equation_id = constraints.constraints_data[constraint_data_id][2];
  std::vector<int> equation_data_ids = constraints.get_equation_data_ids_from_equation_constraint_id(equation_id);

  for (int data_id : equation_data_ids)
  {
    const auto& equation = constraints.equation_data[data_id];
    int node_id = static_cast<int>(equation[1]);

    if (CubitInterface::get_node_exists(node_id))
    {
      std::array<double, 3> coords = CubitInterface::get_nodal_coordinates(node_id);
      draw_data.push_back({coords[0], coords[1], coords[2], equation[2]});
    }
  }

  return draw_data;
}

std::vector<std::vector<double>> CoreDrawConstraintData::get_draw_data_for_equationgroup(CoreConstraints& constraints, int id)
{
  std::vector<std::vector<double>> draw_data;
  int constraint_data_id = constraints.get_constraints_data_id_from_constraint_id(id);

  if (constraint_data_id == -1 || constraints.constraints_data[constraint_data_id][1] != 4)
  {
    return draw_data;
  }

  int equation_group_id = constraints.constraints_data[constraint_data_id][2];
  std::vector<int> equationgroup_data_ids = constraints.get_equation_group_data_ids_from_equation_group_constraint_id(equation_group_id);

  for (int data_id : equationgroup_data_ids)
  {
    const auto& equation = constraints.equation_group_data[data_id];
    int first_node_id = static_cast<int>(equation[1]);
    int second_node_id = static_cast<int>(equation[2]);

    if (CubitInterface::get_node_exists(first_node_id) && CubitInterface::get_node_exists(second_node_id))
    {
      std::array<double, 3> first_coords = CubitInterface::get_nodal_coordinates(first_node_id);
      std::array<double, 3> second_coords = CubitInterface::get_nodal_coordinates(second_node_id);

      draw_data.push_back({first_coords[0], first_coords[1], first_coords[2], equation[3]});
      draw_data.push_back({second_coords[0], second_coords[1], second_coords[2], equation[3]});
    }
  }

  return draw_data;
}
