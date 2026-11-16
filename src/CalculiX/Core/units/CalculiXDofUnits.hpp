#pragma once

#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

class CalculiXDofUnits
{
public:
  using Boundaries = std::map<std::pair<int, int>, std::pair<int, double>>;

  // Components 1–3 always mean translation; 4–6 always mean rotation.
  // A ROT NODE represents rotations using solver DOFs 1–3 instead of 4–6.
  static bool solver_dof(int component, bool rotation_node, double value, int& dof, std::string& error);
  static bool equation_quantity(int dof, bool rotation_node, std::string& quantity, std::string& error);
  // Modes: 0 free, 1 prescribed, 2 fixed. Identical zero supports coalesce;
  // contradictory prescriptions fail before any export text is emitted.
  static bool boundaries(const std::vector<int>& nodes, const std::set<int>& rotation_nodes,
    const std::vector<double>& values, const std::vector<int>& modes, Boundaries& output, std::string& error);
};
