#include "CalculiXDrawLabels.hpp"
#include "CalculiXDrawInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <sstream>
#include <utility>

namespace
{
bool identifier(const std::string& value, int& output)
{
  const auto parsed = std::from_chars(value.data(), value.data() + value.size(), output);
  return parsed.ec == std::errc{} && parsed.ptr == value.data() + value.size() && output > 0;
}

void sample(DrawLabelData& data, const std::vector<double>& point)
{
  if (point.size() < 3 || !std::isfinite(point[0]) || !std::isfinite(point[1]) || !std::isfinite(point[2]))
  {
    return;
  }
  const std::array<double, 3> anchor = {point[0], point[1], point[2]};
  // One deterministic actual target point per entity, not a decorative glyph
  // vertex or a centroid that may lie outside a disconnected target set.
  if (data.anchors.empty())
  {
    data.anchors.push_back(anchor);
  }
  else if (anchor < data.anchors.front())
  {
    data.anchors.front() = anchor;
  }
}

void nodes(DrawLabelData& data, const std::vector<int>& ids)
{
  for (int id : ids)
  {
    const auto p = CubitInterface::get_nodal_coordinates(id);
    sample(data, {p[0], p[1], p[2]});
  }
}

void element(DrawLabelData& data, int id)
{
  CalculiXMeshInterface mesh;
  auto type = CubitInterface::get_element_type(id);
  if (type != "hex" && type != "tet" && type != "wedge" && type != "pyramid" && type != "face" && type != "edge")
  {
    type = mesh.get_cubit_element_type_entity(type);
  }
  nodes(data, CubitInterface::get_connectivity(type, CubitInterface::get_element_type_id(id)));
}
}

bool CalculiXDrawLabels::read(DrawEntityKey entityKey, DrawLabelData& output, std::string& error, bool targets)
{
  CalculiXLoadsInterface loads;
  CalculiXBCsInterface bcs;
  CalculiXMeshInterface mesh;
  CalculiXDrawInterface drawing;
  DrawLabelData data;
  data.entity = entityKey;
  std::string targetKind, targetId;
  switch (entityKey.feature)
  {
    case DrawFeature::Force:
    {
      LoadsForcesSettings settings;
      if (!loads.get_force_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Force";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "nodeset";
      targetId = settings.nodeset;
      data.values = {{"FX", settings.component1}, {"FY", settings.component2}, {"FZ", settings.component3}, {"MX", settings.component4}, {"MY", settings.component5}, {"MZ", settings.component6}};
      break;
    }
    case DrawFeature::Pressure:
    {
      LoadsPressuresSettings settings;
      if (!loads.get_pressure_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Pressure";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "sideset";
      targetId = settings.sideset;
      data.values = {{"P", settings.magnitude}};
      break;
    }
    case DrawFeature::Heatflux:
    {
      LoadsHeatfluxesSettings settings;
      if (!loads.get_heatflux_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Heat flux";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "sideset";
      targetId = settings.sideset;
      data.values = {{"Flux", settings.magnitude}};
      break;
    }
    case DrawFeature::Gravity:
    {
      LoadsGravitySettings settings;
      if (!loads.get_gravity_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Gravity";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "block";
      targetId = settings.block;
      data.values = {{"Magnitude", settings.magnitude}, {"Direction", settings.direction}};
      break;
    }
    case DrawFeature::Centrifugal:
    {
      LoadsCentrifugalSettings settings;
      if (!loads.get_centrifugal_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Centrifugal";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "block";
      targetId = settings.block;
      data.values = {{"Magnitude", settings.magnitude}, {"Axis", settings.direction}, {"Origin", settings.coordinate}};
      break;
    }
    case DrawFeature::Film:
    {
      LoadsFilmSettings settings;
      if (!loads.get_film_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Film";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "sideset";
      targetId = settings.sideset;
      data.secondaryAmplitude = settings.secondaryAmplitude;
      data.values = {{"Temperature", settings.temperature}, {"Coefficient", settings.coefficient}};
      break;
    }
    case DrawFeature::Radiation:
    {
      LoadsRadiationSettings settings;
      if (!loads.get_radiation_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Radiation";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "sideset";
      targetId = settings.sideset;
      data.secondaryAmplitude = settings.secondaryAmplitude;
      data.values = {{"Temperature", settings.temperature}, {"Coefficient", settings.coefficient}};
      break;
    }
    case DrawFeature::SurfaceTraction:
    {
      LoadsSurfaceTractionSettings settings;
      if (!loads.get_surfacetraction_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Surface traction";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "sideset";
      targetId = settings.sideset;
      data.values = {{"TX", settings.component1}, {"TY", settings.component2}, {"TZ", settings.component3}};
      break;
    }
    case DrawFeature::BodyHeatflux:
    {
      LoadsBodyHeatfluxSettings settings;
      if (!loads.get_bodyheatflux_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Body heat flux";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      data.values = {{"Flux", settings.magnitude}};
      if (targets)
      {
        const auto ids = drawing.get_draw_data_for_load_bodyheatflux(entityKey.id);
        if (!ids.empty())
        {
          for (int block : ids[0])
          {
            nodes(data, mesh.get_block_node_ids(block));
          }
        }
        if (ids.size() > 1)
        {
          for (int id : ids[1])
          {
            element(data, id);
          }
        }
      }
      break;
    }
    case DrawFeature::Displacement:
    {
      BCsDisplacementsSettings settings;
      if (!bcs.get_displacement_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Displacement BC";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "nodeset";
      targetId = settings.nodeset;
      data.values = {{"UX", settings.component1}, {"UY", settings.component2}, {"UZ", settings.component3}, {"RX", settings.component4}, {"RY", settings.component5}, {"RZ", settings.component6}};
      break;
    }
    case DrawFeature::Temperature:
    {
      BCsTemperaturesSettings settings;
      if (!bcs.get_temperature_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Temperature BC";
      data.name = settings.name;
      data.amplitude = settings.amplitude;
      targetKind = "nodeset";
      targetId = settings.nodeset;
      data.values = {{"Temperature", settings.magnitude}};
      break;
    }
    case DrawFeature::Trajectory:
    {
      LoadsTrajectorySettings settings;
      if (!loads.get_trajectory_settings(entityKey.id, settings, error))
      {
        return false;
      }
      data.typeName = "Trajectory";
      data.name = settings.name;
      data.values = {{"Type", settings.loadType}, {"Begin", settings.timeBegin}, {"End", settings.timeEnd}};
      if (targets)
      {
        if (settings.loadType != "Heatflux" && settings.loadType != "BodyHeatfluxSphere")
        {
          error = "Unsupported trajectory drawing label target type.";
          return false;
        }
        const auto ids = settings.loadType == "Heatflux" ? loads.loadstrajectory_heatflux_get_node_ids(entityKey.id) :
          loads.loadstrajectory_bodyheatfluxsphere_get_node_ids(entityKey.id);
        nodes(data, ids);
      }
      break;
    }
    default:
      error = "Labels are supported for loads and boundary conditions only.";
      return false;
  }
  if (targets && !targetKind.empty())
  {
    int id = 0;
    if (!identifier(targetId, id))
    {
      error = "Invalid drawing label target ID.";
      return false;
    }
    if (targetKind == "block")
    {
      nodes(data, mesh.get_block_node_ids(id));
    }
    else
    {
      const auto points = targetKind == "nodeset" ? mesh.get_nodeset_entities_coords(id) : mesh.get_sideset_entities_coords_normals(id);
      for (const auto& point : points)
      {
        sample(data, point);
      }
    }
  }
  output = std::move(data);
  error.clear();
  return true;
}

std::string CalculiXDrawLabels::text(const DrawLabelData& data, const DrawLabelSettings& settings)
{
  std::string result;
  const auto add = [&](const std::string& value)
  {
    if (!value.empty())
    {
      if (!result.empty())
      {
        result += " | ";
      }
      result += value;
    }
  };
  if (settings.showId)
  {
    add(data.typeName + " " + std::to_string(data.entity.id));
  }
  if (settings.showName)
  {
    add(data.name);
  }
  if (settings.showValues)
  {
    for (const auto& value : data.values)
    {
      if (!value.second.empty())
      {
        add(value.first + "=" + value.second);
      }
    }
  }
  if (settings.showAmplitude && !data.amplitude.empty())
  {
    add("Amplitude " + data.amplitude + " (reference)");
  }
  if (settings.showAmplitude && !data.secondaryAmplitude.empty())
  {
    add("Secondary amplitude " + data.secondaryAmplitude + " (reference)");
  }
  if (!result.empty() && data.stepId >= 0)
  {
    result = "Step " + std::to_string(data.stepId) + " | " + result;
  }
  // Native label primitives are single-line annotations; names never introduce
  // controls or command text. Values are definitions, not solver output units.
  for (char& character : result)
  {
    if (static_cast<unsigned char>(character) < 32 || character == 127)
    {
      character = ' ';
    }
  }
  return result;
}

bool CalculiXDrawLabels::append(const DrawLabelData& data, const DrawLabelSettings& settings, double symbolSize,
  DrawingGeometry& output, std::string& error)
{
  if (!settings.validate(error) || !std::isfinite(symbolSize) || symbolSize <= 0)
  {
    if (error.empty())
    {
      error = "Invalid drawing label symbol size.";
    }
    return false;
  }
  const auto label = text(data, settings);
  if (!settings.enabled || label.empty() || data.anchors.empty())
  {
    error.clear();
    return true;
  }
  DrawingGeometry candidate = output;
  // Point coordinates and symbolSize use model world units. The displacement
  // length is offsetFactor*symbolSize, independent of viewport size or camera.
  const double offset = settings.offsetFactor * symbolSize / std::sqrt(3.0);
  for (const auto& anchor : data.anchors)
  {
    const std::vector<double> target = {anchor[0], anchor[1], anchor[2]};
    const std::vector<double> position = {anchor[0] + offset, anchor[1] + offset, anchor[2] + offset};
    if ((settings.leaderLines && !candidate.line(target, position, settings.color)) || !candidate.label(position, label, settings.color))
    {
      error = "Cannot create a finite drawing label and leader.";
      return false;
    }
  }
  output = std::move(candidate);
  error.clear();
  return true;
}

void CalculiXDrawLabels::resolved_value(DrawLabelData& data, const std::string& component, const std::string& value)
{
  for (auto& existing : data.values)
  {
    if (existing.first == component)
    {
      if (existing.second != value)
      {
        existing.second = "varies by target";
      }
      return;
    }
  }
  data.values.emplace_back(component, value);
}
