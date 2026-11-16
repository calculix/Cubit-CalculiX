#include "CalculiXDofUnits.hpp"
#include <cmath>

bool CalculiXDofUnits::solver_dof(int component, bool rotation_node, double value, int& dof, std::string& error)
{
  error.clear();
  if (component < 1 || component > 6 || !std::isfinite(value))
  {
    error = "Invalid mechanical component.";
    return false;
  }
  dof = component;
  if (rotation_node)
  {
    if (component <= 3)
    {
      if (value != 0.0)
      {
        error = "A rigid-body rotation node requires moment/rotation components 4–6.";
        return false;
      }
    }
    else
    {
      dof = component - 3;
    }
  }
  return true;
}

bool CalculiXDofUnits::equation_quantity(int dof, bool rotation_node, std::string& quantity, std::string& error)
{
  error.clear();
  quantity.clear();
  if (rotation_node)
  {
    if (dof < 1 || dof > 3)
    {
      error = "A rigid-body rotation node supports equation DOFs 1–3.";
      return false;
    }
    quantity = "angle";
  }
  else if (dof >= 1 && dof <= 3)
  {
    quantity = "length";
  }
  else if (dof >= 4 && dof <= 6)
  {
    quantity = "angle";
  }
  else if (dof == 11)
  {
    quantity = "temperature_difference";
  }
  else
  {
    error = "Unsupported equation degree of freedom.";
    return false;
  }
  return true;
}

bool CalculiXDofUnits::boundaries(const std::vector<int>& nodes, const std::set<int>& rotation_nodes,
  const std::vector<double>& values, const std::vector<int>& modes, Boundaries& output, std::string& error)
{
  error.clear();
  if (nodes.empty() || values.size() != 6 || modes.size() != 6)
  {
    error = "Incomplete mechanical boundary targets or components.";
    return false;
  }
  Boundaries candidate;
  for (int node : nodes)
  {
    if (node <= 0)
    {
      error = "Invalid mechanical boundary node.";
      return false;
    }
    for (int component = 1; component <= 6; ++component)
    {
      const int mode = modes[component - 1];
      if (mode < 0 || mode > 2)
      {
        error = "Invalid mechanical boundary mode.";
        return false;
      }
      if (mode == 0)
      {
        continue;
      }
      double value = 0.0;
      if (mode == 1)
      {
        value = values[component - 1];
      }
      int dof = 0;
      if (!solver_dof(component, rotation_nodes.count(node) != 0, value, dof, error))
      {
        return false;
      }
      const auto inserted = candidate.emplace(std::make_pair(node, dof), std::make_pair(mode, value));
      if (!inserted.second)
      {
        auto& previous = inserted.first->second;
        if (previous.second != value)
        {
          error = "Conflicting mechanical prescriptions on node " + std::to_string(node) + ", DOF " + std::to_string(dof) + ".";
          return false;
        }
        if (mode == 2)
        {
          previous.first = 2;
        }
      }
    }
  }
  output = std::move(candidate);
  return true;
}
