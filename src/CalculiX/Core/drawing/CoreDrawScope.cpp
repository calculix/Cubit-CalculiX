#include "Core/drawing/CalculiXDrawingAdapter.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include <cctype>
#include "Core/application/CoreIdSelection.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include <exception>
#include "CoreDrawScope.hpp"
#include "CalculiXDrawLabels.hpp"
#include "Core/CalculiXCore.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "MeshExportInterface.hpp"
#include "CubitInterface.hpp"
#include "Framework/Drawing/DrawingSymbols.hpp"
#include "CoreDrawMeshGeometry.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <set>
#include <sstream>
#include <tuple>

namespace
{
struct Condition
{
  DrawEntityKey owner;
  std::vector<double> point, direction;
  double value = 0;
  int dof = 0;
  int step = -1;
  int element = -1;
  bool body = false;
  std::string color;
  std::string labelValue;
  bool homogeneous = false;
};

// Native keyword families are independent. OP=NEW only applies to the first
// occurrence of a family in a step. CLOAD/DLOAD contributions in one step add.
// CalculiX manual: ccx_2.18/doc/ccx/node229.html and node257.html.
class Conditions
{
public:
  std::map<std::string, std::map<std::string, Condition>> values;
  std::set<std::string> seen;
  void beginStep()
  {
    seen.clear();
  }
  void beginCard(const std::string& family, bool reset)
  {
    if (seen.insert(family).second && reset)
    {
      values[family].clear();
    }
  }
  void put(const std::string& family, const std::string& address, Condition condition, bool additive)
  {
    auto& entries = values[family];
    const auto previous = entries.find(address);
    if (additive && previous != entries.end() && previous->second.step == condition.step)
    {
      condition.value += previous->second.value;
    }
    entries[address] = std::move(condition);
  }
};

std::vector<double> coordinate(int node)
{
  const auto p = CubitInterface::get_nodal_coordinates(node);
  return {p[0], p[1], p[2]};
}

std::vector<double> numbers(const std::string& text)
{
  std::istringstream stream(text);
  std::vector<double> values;
  double value = 0;
  while (stream >> value)
  {
    values.push_back(value);
  }
  return values;
}

bool center(int element, std::vector<double>& point)
{
  CalculiXMeshInterface mesh;
  std::string type = CubitInterface::get_element_type(element);
  if (type != "hex" && type != "tet" && type != "wedge" && type != "pyramid" && type != "face" && type != "edge")
  {
    type = mesh.get_cubit_element_type_entity(type);
  }
  const int localId = CubitInterface::get_element_type_id(element);
  const auto nodes = CubitInterface::get_connectivity(type, localId);
  if (nodes.empty())
  {
    return false;
  }
  point.assign(3, 0);
  for (int node : nodes)
  {
    const auto p = coordinate(node);
    for (int axis = 0; axis < 3; ++axis)
    {
      point[axis] += p[axis] / static_cast<double>(nodes.size());
    }
  }
  return true;
}

bool faceSample(MeshExportInterface& mesh, ElementHandle element, ElementType type, int side, std::vector<double>& point, std::vector<double>& normal)
{
  int nodeCount = 0, sideCount = 0;
  // Same zero-based side-index convention as CoreResultsVtkWriter's linking path.
  if (side <= 0 || !mesh.get_nodes_per_element(type, nodeCount) || !mesh.get_nodes_per_side(type, side - 1, sideCount) || nodeCount < 2 || sideCount < 2)
  {
    return false;
  }
  std::vector<int> nodes(nodeCount), indices(sideCount);
  if (mesh.get_connectivity(element, nodes) != nodeCount || !mesh.get_node_indices_per_side(type, side - 1, indices))
  {
    return false;
  }
  std::vector<std::vector<double>> face;
  point.assign(3, 0);
  std::vector<double> centroid(3, 0);
  for (int node : nodes)
  {
    const auto p = coordinate(node);
    for (int axis = 0; axis < 3; ++axis)
    {
      centroid[axis] += p[axis] / static_cast<double>(nodes.size());
    }
  }
  for (int index : indices)
  {
    if (index < 0 || index >= nodeCount)
    {
      return false;
    }
    const auto p = coordinate(nodes[index]);
    face.push_back(p);
    for (int axis = 0; axis < 3; ++axis)
    {
      point[axis] += p[axis] / static_cast<double>(indices.size());
    }
  }
  const auto difference = [](const std::vector<double>& a, const std::vector<double>& b)
  {
    return std::vector<double>{a[0]-b[0], a[1]-b[1], a[2]-b[2]};
  };
  if (face.size() >= 3)
  {
    normal = DrawingSymbols::cross_product(difference(face[1], face[0]), difference(face[2], face[0]));
  }
  else if (nodes.size() >= 3)
  {
    const auto a = coordinate(nodes[0]), b = coordinate(nodes[1]), c = coordinate(nodes[2]);
    const auto plane = DrawingSymbols::cross_product(difference(b, a), difference(c, a));
    normal = DrawingSymbols::cross_product(difference(face[1], face[0]), plane);
  }
  else
  {
    return false;
  }
  if (DrawingSymbols::inner_product(normal, difference(point, centroid)) < 0)
  {
    normal = DrawingSymbols::mult_vector_scalar(normal, -1);
  }
  if (DrawingSymbols::magnitude(normal) <= 1e-15)
  {
    return false;
  }
  normal = DrawingSymbols::unit_vector(normal);
  return true;
}

bool boundary(CalculiXCore& core, Conditions& conditions, int type, int id, int step, bool homogeneous, std::string& error)
{
  int nodeset = -1;
  bool reset = false;
  std::string family = "BOUNDARY";
  std::vector<int> dofs;
  std::map<int, std::string> componentValues;
  if (type == 1)
  {
    BCsDisplacementsSettings data;
    if (!core.bcsdisplacements->get_settings(id, data, error))
    {
      return false;
    }
    nodeset = std::stoi(data.nodeset);
    reset = data.operation == "new";
    const std::string components[] = {data.component1, data.component2, data.component3, data.component4, data.component5, data.component6};
    for (int axis = 0; axis < 6; ++axis)
    {
      if (components[axis] != "free")
      {
        dofs.push_back(axis + 1);
        componentValues[axis + 1] = homogeneous ? "0" : components[axis];
      }
    }
  }
  else
  {
    BCsTemperaturesSettings data;
    if (!core.bcstemperatures->get_settings(id, data, error))
    {
      return false;
    }
    nodeset = std::stoi(data.nodeset);
    reset = data.operation == "new";
    family = data.keyword == "*TEMPERATURE" && !homogeneous ? "TEMPERATURE" : "BOUNDARY";
    dofs = {11};
    componentValues[11] = homogeneous ? "0" : data.magnitude;
  }
  conditions.beginCard(family, !homogeneous && reset);
  for (int node : CubitInterface::get_nodeset_nodes_inclusive(nodeset))
  {
    for (int dof : dofs)
    {
      Condition condition;
      condition.owner = {type == 1 ? DrawFeature::Displacement : DrawFeature::Temperature, id};
      condition.point = coordinate(node);
      condition.dof = dof;
      condition.labelValue = componentValues[dof];
      condition.homogeneous = homogeneous;
      condition.step = step;
      condition.color = type == 1 ? "coral" : "red";
      conditions.put(family, std::to_string(node) + ":" + std::to_string(dof), condition, false);
    }
  }
  return true;
}

bool force(CalculiXCore& core, Conditions& conditions, int id, int step, std::string& error)
{
  LoadsForcesSettings data;
  if (!core.loadsforces->get_settings(id, data, error))
  {
    return false;
  }
  conditions.beginCard("CLOAD", data.operation == "new");
  const std::string components[] = {data.component1, data.component2, data.component3, data.component4, data.component5, data.component6};
  for (int node : CubitInterface::get_nodeset_nodes_inclusive(std::stoi(data.nodeset)))
  {
    for (int axis = 0; axis < 6; ++axis)
    {
      const double value = std::stod(components[axis]);
      if (value == 0)
      {
        continue;
      }
      Condition condition;
      condition.owner = {DrawFeature::Force, id};
      condition.point = coordinate(node);
      condition.direction.assign(3, 0);
      condition.direction[axis % 3] = 1;
      condition.value = value;
      condition.dof = axis + 1;
      condition.step = step;
      condition.color = axis < 3 ? "green" : "seagreen";
      conditions.put("CLOAD", std::to_string(node) + ":" + std::to_string(axis + 1), condition, true);
    }
  }
  return true;
}

bool surface(CalculiXCore& core, Conditions& conditions, DrawFeature feature, int id, int sideset, bool reset, double magnitude, const std::string& family, int step, const std::string& color, std::string& error)
{
  conditions.beginCard(family, reset);
  SidesetHandle handle;
  if (!core.me_iface->get_sideset_handle(sideset, handle))
  {
    error = "A step load references a missing sideset.";
    return false;
  }
  constexpr int count = 256;
  std::vector<ElementHandle> elements(count);
  std::vector<ElementType> types(count);
  std::vector<int> sides(count);
  int offset = 0, received = 0;
  CalculiXMeshInterface mesh;
  while ((received = core.me_iface->get_sideset_sides(handle, offset, count, elements, types, sides)) > 0)
  {
    std::vector<int> ids(received);
    core.me_iface->get_element_ids(received, elements, ids);
    for (int index = 0; index < received; ++index)
    {
      const auto entity = mesh.get_cubit_element_type_entity(core.me_iface->get_element_type_name(types[index]));
      const int global = CubitInterface::get_global_element_id(entity, ids[index]);
      Condition condition;
      if (!faceSample(*core.me_iface, elements[index], types[index], sides[index], condition.point, condition.direction))
      {
        error = "Unable to resolve a loaded element face for step drawing.";
        return false;
      }
      condition.owner = {feature, id};
      condition.value = magnitude;
      condition.step = step;
      condition.color = color;
      const int side = mesh.get_ccx_element_side(static_cast<int>(types[index]), sides[index]);
      conditions.put(family, std::to_string(global) + ":" + std::to_string(side), condition, family == "DLOAD" || family == "DFLUX");
    }
    offset += received;
  }
  return received >= 0;
}

bool body(CalculiXCore& core, Conditions& conditions, DrawFeature feature, int id, const std::vector<int>& elements, bool reset, double magnitude, const std::vector<double>& direction, const std::vector<double>& origin, int step, std::string& error)
{
  const std::string family = feature == DrawFeature::BodyHeatflux ? "DFLUX" : "DLOAD";
  conditions.beginCard(family, reset);
  for (int element : elements)
  {
    Condition condition;
    if (!center(element, condition.point))
    {
      error = "Unable to resolve a body-load element for step drawing.";
      return false;
    }
    condition.owner = {feature, id};
    condition.direction = direction;
    if (feature == DrawFeature::Centrifugal)
    {
      if (origin.size() != 3 || direction.size() != 3)
      {
        return false;
      }
      const auto axis = DrawingSymbols::unit_vector(direction);
      const std::vector<double> radial = {
        condition.point[0] - origin[0],
        condition.point[1] - origin[1],
        condition.point[2] - origin[2]
      };

      // Remove the component parallel to the normalized rotation axis:
      // r_perpendicular = r - (r . axis) axis.
      const double axialDistance = DrawingSymbols::inner_product(radial, axis);
      condition.direction.resize(3);
      for (std::size_t component = 0; component < 3; ++component)
      {
        condition.direction[component] =
          radial[component] - axialDistance * axis[component];
      }
    }
    condition.value = magnitude;
    condition.step = step;
    condition.body = feature == DrawFeature::BodyHeatflux;
    condition.element = element;
    condition.color = condition.body ? "purple" : (feature == DrawFeature::Gravity ? "brown" : "khaki");
    std::string address = std::to_string(element) + (condition.body ? ":BF" : ":BODY:");
    if (!condition.body)
    {
      for (double value : direction)
      {
        address += std::to_string(value) + ":";
      }
      for (double value : origin)
      {
        address += std::to_string(value) + ":";
      }
    }
    conditions.put(family, address, condition, true);
  }
  return true;
}

bool traction(CalculiXCore& core, Conditions& conditions, int id, int step, std::string& error)
{
  LoadsSurfaceTractionSettings data;
  if (!core.loadssurfacetraction->get_settings(id, data, error))
  {
    return false;
  }
  conditions.beginCard("CLOAD", data.operation == "new");
  std::istringstream stream(core.loadssurfacetraction->get_load_export(id));
  std::string line;
  while (std::getline(stream, line))
  {
    if (line.empty() || line[0] == '*')
    {
      continue;
    }
    std::replace(line.begin(), line.end(), ',', ' ');
    std::istringstream row(line);
    int node = 0, dof = 0;
    double value = 0;
    if (!(row >> node >> dof >> value) || dof < 1 || dof > 6)
    {
      error = "Unable to resolve surface-traction nodal forces.";
      return false;
    }
    Condition condition;
    condition.owner = {DrawFeature::SurfaceTraction, id};
    condition.point = coordinate(node);
    condition.direction.assign(3, 0);
    condition.direction[(dof - 1) % 3] = 1;
    condition.value = value;
    condition.dof = dof;
    condition.step = step;
    condition.color = "darkgreen";
    conditions.put("CLOAD", std::to_string(node) + ":" + std::to_string(dof), condition, true);
  }
  return true;
}
}

bool CoreDrawScope::resolveModel(CalculiXCore& core, ResolvedDrawScope& result, std::string& error)
{
  result = {};
  error.clear();
  for (int id : core.loadsforces->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Force, id});
  }
  for (int id : core.loadspressures->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Pressure, id});
  }
  for (int id : core.loadsheatfluxes->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Heatflux, id});
  }
  for (int id : core.loadsgravity->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Gravity, id});
  }
  for (int id : core.loadscentrifugal->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Centrifugal, id});
  }
  for (int id : core.loadstrajectory->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Trajectory, id});
  }
  for (int id : core.loadsfilm->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Film, id});
  }
  for (int id : core.loadsradiation->get_load_ids())
  {
    result.entities.push_back({DrawFeature::Radiation, id});
  }
  for (int id : core.loadssurfacetraction->get_load_ids())
  {
    result.entities.push_back({DrawFeature::SurfaceTraction, id});
  }
  for (int id : core.loadsbodyheatflux->get_load_ids())
  {
    result.entities.push_back({DrawFeature::BodyHeatflux, id});
  }
  for (int id : core.bcsdisplacements->get_bc_ids())
  {
    result.entities.push_back({DrawFeature::Displacement, id});
  }
  for (int id : core.bcstemperatures->get_bc_ids())
  {
    result.entities.push_back({DrawFeature::Temperature, id});
  }
  for (int id : CoreIdSelection::collect_ids(core.orientations->orientations_data))
  {
    result.entities.push_back({DrawFeature::Orientation, id});
  }
  for (int id : core.constraints->get_equation_ids())
  {
    result.entities.push_back({DrawFeature::Equation, id});
  }
  for (int id : core.constraints->get_equationgroup_ids())
  {
    result.entities.push_back({DrawFeature::EquationGroup, id});
  }
  return true;
}

bool CoreDrawScope::resolveStep(CalculiXCore& core, int stepId, double size, ResolvedDrawScope& result, std::string& error)
{
  result = {};
  error.clear();
  if (!core.steps->check_step_exists(stepId))
  {
    error = "The selected drawing step no longer exists.";
    return false;
  }
  // Native reference symbols are not a time-dependent solver evaluation.
  // Raw custom condition cards cannot be resolved from the typed feature data.
  for (const auto& custom : core.customlines->customlines_data)
  {
    if (custom.size() < 6)
    {
      continue;
    }
    std::string text = custom[5];
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    std::istringstream lines(text);
    std::string line;
    while (std::getline(lines, line))
    {
      const auto first = line.find_first_not_of(" \t\r");
      if (first == std::string::npos || line.compare(first, 2, "**") == 0)
      {
        continue;
      }
      for (const auto* keyword : {"*BOUNDARY", "*CLOAD", "*DLOAD", "*DFLUX", "*FILM", "*RADIATE", "*TEMPERATURE", "*MODEL CHANGE", "*TRANSFORM", "*STEP"})
      {
        if (line.compare(first, std::char_traits<char>::length(keyword), keyword) == 0)
        {
          error = "Step reference drawing cannot resolve condition overrides in custom line " + custom[0] + ". Use the entire-model or explicit-ID drawing instead.";
          return false;
        }
      }
    }
  }

  try
  {
    Conditions conditions;
    for (const auto& assignment : core.hbcs->bcs_data)
    {
      if (assignment.size() >= 3 && !boundary(core, conditions, assignment[1], assignment[2], -1, true, error))
      {
        return false;
      }
    }
    CalculiXMeshInterface mesh;
    int ordinal = 0;
    bool found = false;
    for (const auto& step : core.steps->steps_data)
    {
      if (step.size() < 9)
      {
        error = "Incomplete stored step data.";
        return false;
      }
      StepsSettings settings;
      if (!core.steps->get_settings(step[0], settings, error))
      {
        return false;
      }
      const bool perturbation = (settings.parameters.size() > 1 && settings.parameters[1] == "YES") || settings.type == 4 || settings.type == 9 || settings.type == 10;
      const auto clearLoading = [&]()
      {
        for (const auto* family : {"CLOAD", "DLOAD", "DFLUX", "FILM", "RADIATE", "TEMPERATURE"})
        {
          conditions.values[family].clear();
        }
      };
      if (perturbation)
      {
        clearLoading();
      }
      conditions.beginStep();
      const auto assigned = [&](int type, int id)
      {
        for (const auto& row : core.steps->loads_data)
        {
          if (row.size() >= 3 && row[0] == step[5] && row[1] == type && row[2] == id)
          {
            return true;
          }
        }
        return false;
      };
      for (int id : core.loadsforces->get_load_ids())
      {
        if (!assigned(1, id))
        {
          continue;
        }
        LoadsForcesSettings data;
        if (!core.loadsforces->get_settings(id, data, error) || !force(core, conditions, data.id, ordinal, error))
        {
          return false;
        }
      }
      for (int id : core.loadspressures->get_load_ids())
      {
        if (!assigned(2, id))
        {
          continue;
        }
        LoadsPressuresSettings data;
        if (!core.loadspressures->get_settings(id, data, error) || !surface(core, conditions, DrawFeature::Pressure, data.id, std::stoi(data.sideset), data.operation == "new", std::stod(data.magnitude), "DLOAD", ordinal, "lightskyblue", error))
        {
          return false;
        }
      }
      for (int id : core.loadsheatfluxes->get_load_ids())
      {
        if (!assigned(3, id))
        {
          continue;
        }
        LoadsHeatfluxesSettings data;
        if (!core.loadsheatfluxes->get_settings(id, data, error) || !surface(core, conditions, DrawFeature::Heatflux, data.id, std::stoi(data.sideset), data.operation == "new", std::stod(data.magnitude), "DFLUX", ordinal, "purple", error))
        {
          return false;
        }
      }
      for (int id : core.loadsgravity->get_load_ids())
      {
        if (!assigned(4, id))
        {
          continue;
        }
        LoadsGravitySettings data;
        if (!core.loadsgravity->get_settings(id, data, error) || !body(core, conditions, DrawFeature::Gravity, data.id, mesh.get_block_element_ids(std::stoi(data.block)), data.operation == "new", std::stod(data.magnitude), numbers(data.direction), {}, ordinal, error))
        {
          return false;
        }
      }
      for (int id : core.loadscentrifugal->get_load_ids())
      {
        if (!assigned(5, id))
        {
          continue;
        }
        LoadsCentrifugalSettings data;
        if (!core.loadscentrifugal->get_settings(id, data, error) || !body(core, conditions, DrawFeature::Centrifugal, data.id, mesh.get_block_element_ids(std::stoi(data.block)), data.operation == "new", std::stod(data.magnitude), numbers(data.direction), numbers(data.coordinate), ordinal, error))
        {
          return false;
        }
      }
      for (int id : core.loadsfilm->get_load_ids())
      {
        if (!assigned(7, id))
        {
          continue;
        }
        LoadsFilmSettings data;
        if (!core.loadsfilm->get_settings(id, data, error) || !surface(core, conditions, DrawFeature::Film, data.id, std::stoi(data.sideset), data.operation == "new", 1.0, "FILM", ordinal, "indianred", error))
        {
          return false;
        }
      }
      for (int id : core.loadsradiation->get_load_ids())
      {
        if (!assigned(8, id))
        {
          continue;
        }
        LoadsRadiationSettings data;
        if (!core.loadsradiation->get_settings(id, data, error) || !surface(core, conditions, DrawFeature::Radiation, data.id, std::stoi(data.sideset), data.operation == "new", 1.0, "RADIATE", ordinal, "tomato", error))
        {
          return false;
        }
      }
      for (int id : core.loadssurfacetraction->get_load_ids())
      {
        if (!assigned(9, id))
        {
          continue;
        }
        LoadsSurfaceTractionSettings data;
        if (!core.loadssurfacetraction->get_settings(id, data, error) || !traction(core, conditions, data.id, ordinal, error))
        {
          return false;
        }
      }
      for (int id : core.loadsbodyheatflux->get_load_ids())
      {
        if (!assigned(10, id))
        {
          continue;
        }
        LoadsBodyHeatfluxSettings data;
        if (!core.loadsbodyheatflux->get_settings(id, data, error))
        {
          return false;
        }
        std::vector<int> elements;
        std::istringstream ids(data.ids);
        int entity = 0;
        while (ids >> entity)
        {
          if (data.entityType == "block")
          {
            const auto members = mesh.get_block_element_ids(entity);
            elements.insert(elements.end(), members.begin(), members.end());
          }
          else
          {
            elements.push_back(entity);
          }
        }
        if (!body(core, conditions, DrawFeature::BodyHeatflux, id, elements, data.operation == "new", std::stod(data.magnitude), {}, {}, ordinal, error))
        {
          return false;
        }
      }
      for (int type : {1, 2})
      {
        for (const auto& assignment : core.steps->bcs_data)
        {
          if (assignment.size() >= 3 && assignment[0] == step[6] && assignment[1] == type)
          {
            if (!boundary(core, conditions, type, assignment[2], ordinal, false, error))
            {
              return false;
            }
          }
        }
      }
      if (step[0] == stepId)
      {
        found = true;
        // A trajectory is a path overlay; its generated load history remains solver-owned.
        for (const auto& assignment : core.steps->loads_data)
        {
          if (assignment.size() >= 3 && assignment[0] == step[5] && assignment[1] == 6)
          {
            result.entities.push_back({DrawFeature::Trajectory, assignment[2]});
          }
        }
        break;
      }
      if (perturbation)
      {
        clearLoading();
      }
      ++ordinal;
    }
    if (!found)
    {
      error = "The drawing step was not found in export order.";
      return false;
    }
    for (const auto& family : conditions.values)
    {
      for (const auto& entry : family.second)
      {
        const auto& condition = entry.second;
        auto& geometry = result.geometry[condition.owner];
        auto label = result.labels.find(condition.owner);
        if (label == result.labels.end())
        {
          DrawLabelData data;
          if (!CalculiXDrawLabels::read(condition.owner, data, error, false))
          {
            return false;
          }
          data.values.clear();
          if (condition.homogeneous)
          {
            data.amplitude.clear();
            data.secondaryAmplitude.clear();
            data.typeName += " (homogeneous)";
          }
          data.stepId = stepId;
          data.symbolSize = size;
          label = result.labels.emplace(condition.owner, std::move(data)).first;
        }
        if (label->second.anchors.empty() && condition.point.size() == 3)
        {
          label->second.anchors.push_back({condition.point[0], condition.point[1], condition.point[2]});
        }
        const char* forceComponents[] = {"FX", "FY", "FZ", "MX", "MY", "MZ"};
        const char* boundaryComponents[] = {"UX", "UY", "UZ", "RX", "RY", "RZ"};
        std::string component = "Defined step value";
        if (condition.dof == 11)
        {
          component = "Temperature";
        }
        else if (condition.dof >= 1 && condition.dof <= 6)
        {
          component = family.first == "BOUNDARY" ? boundaryComponents[condition.dof - 1] : forceComponents[condition.dof - 1];
        }
        std::ostringstream value;
        value.precision(8);
        value << condition.value;
        CalculiXDrawLabels::resolved_value(label->second, component, condition.labelValue.empty() ? value.str() : condition.labelValue);
        bool valid = true;
        if (family.first == "BOUNDARY" || family.first == "TEMPERATURE")
        {
          valid = CalculiXDrawingAdapter::displacement(geometry, condition.point, condition.dof, condition.color, size);
        }
        else if (condition.body)
        {
          valid = condition.value == 0 || CoreDrawMeshGeometry::append("element", condition.element, false, condition.color, geometry);
        }
        else if (condition.value != 0)
        {
          const bool towardVector = condition.owner.feature == DrawFeature::Force || condition.owner.feature == DrawFeature::SurfaceTraction || condition.owner.feature == DrawFeature::Gravity || condition.owner.feature == DrawFeature::Centrifugal;
          auto direction = DrawingSymbols::mult_vector_scalar(condition.direction, towardVector ? -condition.value : condition.value);
          if (condition.dof > 3)
          {
            valid = DrawingSymbols::flatArrow(geometry, condition.point, direction, false, condition.color, size);
          }
          else
          {
            valid = DrawingSymbols::arrow(geometry, condition.point, direction, false, condition.color, size);
          }
        }
        if (!valid)
        {
          error = "Unable to build the resolved step drawing.";
          return false;
        }
      }
    }
    for (int id : CoreIdSelection::collect_ids(core.orientations->orientations_data))
    {
      result.entities.push_back({DrawFeature::Orientation, id});
    }
    for (int id : core.constraints->get_equation_ids())
    {
      result.entities.push_back({DrawFeature::Equation, id});
    }
    for (int id : core.constraints->get_equationgroup_ids())
    {
      result.entities.push_back({DrawFeature::EquationGroup, id});
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Invalid stored step drawing data: ") + exception.what();
    return false;
  }
}
