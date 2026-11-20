#include "CoreDiscreteElementExport.hpp"
#include "CoreConnections.hpp"
#include "CoreConnectionEndpointResolver.hpp"
#include "Core/export/CoreExportNodeAllocator.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitInterface.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <utility>

namespace
{
using Position = std::array<double,3>;
using FeatureNodes = std::map<std::pair<std::string,int>,std::pair<int,int>>;

bool enabled(int history, int step)
{
  return history == -1 || history == step;
}

bool mechanical(int procedure)
{
  return procedure == 2 || procedure == 6 || procedure == 7 || procedure == 8;
}

QJsonArray coordinates(const Position& p)
{
  return {p[0],p[1],p[2]};
}

bool validate_mass_nodes(const CoreConnections& model, const CoreExportNodeAllocator& allocator,
  const std::map<int,std::array<int,2>>& roles, const FeatureNodes& used_nodes, std::string& error)
{
  // A MASS element alone does not activate the node's translational equations in CCX.
  std::map<int,int> unsupported;
  for (const auto& item : model.point_masses())
  {
    const int node = used_nodes.at({"MASS",item.first}).first;
    if (!allocator.custom_element_uses(node))
    {
      unsupported.emplace(node,item.first);
    }
  }
  for (const auto& item : used_nodes)
  {
    if (item.first.first != "MASS")
    {
      unsupported.erase(item.second.first);
      unsupported.erase(item.second.second);
    }
  }
  for (const auto& role : roles)
  {
    unsupported.erase(role.second[0]);
  }
  if (!unsupported.empty())
  {
    CalculiXMeshInterface mesh;
    for (int block : mesh.get_blocks())
    {
      if (CubitInterface::get_block_element_type(block) == "SPHERE")
      {
        continue;
      }
      for (int node : mesh.get_block_node_ids(block))
      {
        unsupported.erase(node);
      }
      if (unsupported.empty())
      {
        break;
      }
    }
  }
  if (!unsupported.empty())
  {
    const auto& mass = model.point_masses().at(unsupported.begin()->second);
    error = "Point mass " + mass.name + " needs a node on a structural element, spring or rigid-body REF.";
    return false;
  }
  return true;
}

bool append_history(const CoreConnections& model, CoreSteps& steps, CoreExportNodeAllocator& allocator,
  const FeatureNodes& used_nodes, std::ostream& nodes, std::string& error)
{
  int ordinal = 0;
  bool nonlinear = false;
  for (const auto& step : steps.steps_data)
  {
    ++ordinal;
    if (step.size() < 4)
    {
      error = "Incomplete step data while exporting discrete element history.";
      return false;
    }
    const int parameter = steps.get_parameter_data_id_from_parameter_id(step[2]);
    if (parameter < 0 || steps.parameter_data[parameter].size() < 3)
    {
      error = "Missing step parameters while exporting discrete element history.";
      return false;
    }
    const auto& mode = steps.parameter_data[parameter][2];
    if (mode == "YES" || mode == "NO")
    {
      nonlinear = mode == "YES";
    }
    if (!CoreDiscreteElementExport::needs_history(model, step[0], step[3]))
    {
      continue;
    }
    std::set<int> members;
    for (const auto& item : used_nodes)
    {
      const auto& type = item.first.first;
      const int id = item.first.second;
      const int history = type == "MASS" ? model.point_masses().at(id).history_step : model.springs().at(id).history_step;
      if (enabled(history, step[0]))
      {
        nodes << "** CUBIT_SIM_DISCRETE_HISTORY " << type << " " << id << " " << ordinal << " " << (nonlinear ? 1 : 0) << "\n";
        members.insert(item.second.first);
        if (item.second.second)
        {
          members.insert(item.second.second);
        }
      }
    }
    const auto set = CoreDiscreteElementExport::history_set(ordinal);
    if (!allocator.reserve_set(set))
    {
      error = "Reserved discrete history set is already in use: " + set;
      return false;
    }
    nodes << "*NSET,NSET=" << set << "\n";
    for (int node : members)
    {
      nodes << node << "\n";
    }
  }
  return true;
}
}

std::string CoreDiscreteElementExport::history_set(int ordinal)
{
  return "CCX_DISCRETE_STEP_" + std::to_string(ordinal);
}

bool CoreDiscreteElementExport::needs_history(const CoreConnections& model, int step_id, int procedure)
{
  const auto active = [step_id](const auto& item) { return enabled(item.second.history_step, step_id); };
  return mechanical(procedure) &&
    (std::any_of(model.point_masses().begin(), model.point_masses().end(), active) ||
     std::any_of(model.springs().begin(), model.springs().end(), active));
}

bool CoreDiscreteElementExport::append(const CoreConnections& model, CoreSteps& steps, CoreExportNodeAllocator& allocator,
  const std::set<int>& mesh_nodes, const std::map<int,std::array<int,2>>& roles,
  const std::map<int,Position>& positions, std::ostream& nodes, std::ostream& elements, std::string& error)
{
  if (model.point_masses().empty() && model.springs().empty())
  {
    return true;
  }

  SimUnits::QuantityUnit length;
  if (!framework_units().quantity("length", length, error))
  {
    return false;
  }
  double si_tolerance = 0.0;
  double solver_tolerance = 0.0;
  if (!framework_units().geometry_to_si(CubitInterface::get_merge_tolerance(), si_tolerance, error) ||
      !framework_units().to_solver(si_tolerance, length.dimension, "calculix", solver_tolerance, error))
  {
    return false;
  }
  const CoreConnectionEndpointResolver resolver(model, mesh_nodes, roles, positions, length);

  FeatureNodes used_nodes;
  const auto append_element = [&](const std::string& type, int id, const std::string& name,
    const ConnectionEndpoint& first, const ConnectionEndpoint* second, int direction, double value_si)
  {
    CoreConnectionEndpointResolver::Resolved a{}, b{};
    if (!resolver.resolve(first, a, error) || (second && !resolver.resolve(*second, b, error)))
    {
      error = name + ": " + error;
      return false;
    }
    if (second)
    {
      const auto& p = a.second;
      const auto& q = b.second;
      const double length = std::hypot(q[0]-p[0],q[1]-p[1],q[2]-p[2]);
      const double scale = std::max(std::hypot(p[0],p[1],p[2]),std::hypot(q[0],q[1],q[2]));
      if (a.first == b.first || length <= std::max(solver_tolerance, scale * 8 * std::numeric_limits<double>::epsilon()))
      {
        error = "Spring " + name + " requires distinct nodes with separated coordinates.";
        return false;
      }
    }
    const bool mass = type == "MASS";
    const auto set = std::string(mass ? "CCX_POINT_MASS_" : "CCX_SPRING_") + std::to_string(id);
    int element = 0;
    if (!allocator.next_element(element) || !allocator.reserve_set(set))
    {
      error = "Cannot allocate an element ID or private set for " + name + ".";
      return false;
    }
    SimUnits::QuantityUnit quantity;
    double value = 0.0;
    if (!framework_units().quantity(mass ? "mass" : "force_per_length", quantity, error) ||
        !framework_units().to_solver(value_si, quantity.dimension, "calculix", value, error))
    {
      return false;
    }
    elements << "*ELEMENT,TYPE=" << type << ",ELSET=" << set << "\n" << element << "," << a.first;
    if (second)
    {
      elements << "," << b.first;
    }
    elements << "\n" << (mass ? "*MASS" : "*SPRING") << ",ELSET=" << set << "\n";
    if (!mass)
    {
      if (!second)
      {
        elements << direction;
      }
      elements << "\n";
    }
    elements << std::scientific << std::setprecision(12) << value << "\n";
    QJsonObject binding{{"id",id},{"name",QString::fromStdString(name)},{"type",QString::fromStdString(type)},
      {"element",element},{"a",a.first},{"b",b.first},{"direction",direction},{"value",value},
      {"position_a",coordinates(a.second)}};
    if (second)
    {
      binding.insert("position_b",coordinates(b.second));
    }
    nodes << "** CUBIT_SIM_DISCRETE " << QJsonDocument(binding).toJson(QJsonDocument::Compact).toBase64().toStdString() << "\n";
    used_nodes[{type,id}] = {a.first,b.first};
    return true;
  };
  for (const auto& item : model.point_masses())
  {
    const auto& mass = item.second;
    if (!append_element("MASS",mass.id,mass.name,mass.endpoint,nullptr,0,mass.mass_si))
    {
      return false;
    }
  }
  for (const auto& item : model.springs())
  {
    const auto& spring = item.second;
    const auto* second = spring.second ? &*spring.second : nullptr;
    if (!append_element(second ? "SPRINGA" : "SPRING1", spring.id,spring.name,spring.first,second,spring.direction,spring.stiffness_si))
    {
      return false;
    }
  }
  return validate_mass_nodes(model, allocator, roles, used_nodes, error) &&
    append_history(model, steps, allocator, used_nodes, nodes, error);
}
