#include "CoreDrawEntityGeometry.hpp"
#include "CalculiXDrawInterface.hpp"
#include "CalculiXDrawingAdapter.hpp"
#include "CoreDrawMeshGeometry.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "Framework/Drawing/DrawingSymbols.hpp"
#include "CubitInterface.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <utility>

namespace
{
bool validRows(const std::vector<std::vector<double>>& rows, std::size_t size)
{
  for (const auto& row : rows)
  {
    if (row.size() < size || !std::all_of(row.begin(), row.end(), [](double v)
        {
          return std::isfinite(v);
        }))
    {
      return false;
    }
  }
  return true;
}

bool buildForce(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;
  std::vector<std::vector<double>> draw_data = ccx_draw_iface.get_draw_data_for_load_force(id);
  if (!validRows(draw_data, 6) || draw_data.size() % 2 != 0)
  {
    return false;
  }

  for (size_t i = 0; i + 1 < draw_data.size(); i += 2)
  {
    if (draw_data[i][3] != 0.0 || draw_data[i][4] != 0.0 || draw_data[i][5] != 0.0)
    {
      if (!DrawingSymbols::arrow(geometry,
          {draw_data[i][0], draw_data[i][1], draw_data[i][2]},
          {-draw_data[i][3], -draw_data[i][4], -draw_data[i][5]},
          false, "green", size))
      {
        return false;
      }
    }
    if (draw_data[i + 1][3] != 0.0 || draw_data[i + 1][4] != 0.0 || draw_data[i + 1][5] != 0.0)
    {
      if (!DrawingSymbols::flatArrow(geometry,
          {draw_data[i + 1][0], draw_data[i + 1][1], draw_data[i + 1][2]},
          {-draw_data[i + 1][3], -draw_data[i + 1][4], -draw_data[i + 1][5]},
          false, "seagreen", size))
      {
        return false;
      }
    }
  }
  output = std::move(geometry);
  return true;
}

bool buildPressure(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_load_pressure(id);
  if (!validRows(draw_data, 6))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!DrawingSymbols::arrow(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "lightskyblue", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildHeatflux(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_load_heatflux(id);
  if (!validRows(draw_data, 6))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!DrawingSymbols::arrow(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "purple", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildGravity(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_load_gravity(id);
  if (!validRows(draw_data, 6))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!DrawingSymbols::arrow(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "brown", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildCentrifugal(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_load_centrifugal(id);
  if (!validRows(draw_data, 6))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!DrawingSymbols::arrow(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "khaki", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildTrajectory(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXLoadsInterface loads;
  const auto type = loads.loadstrajectory_get_load_type(id);
  std::vector<int> nodes;
  std::vector<std::vector<double>> hits, times;
  std::vector<std::vector<std::vector<int>>> elements;
  bool faces = false;
  if (type == "HEATFLUX")
  {
    faces = true;
    nodes = loads.loadstrajectory_heatflux_get_node_ids(id);
    hits = loads.loadstrajectory_heatflux_get_hit_coordinates(id);
    times = loads.loadstrajectory_heatflux_get_times(id);
    elements = loads.loadstrajectory_heatflux_get_draw_face_ids(id);
  }
  else if (type == "BODYHEATFLUXSPHERE")
  {
    nodes = loads.loadstrajectory_bodyheatfluxsphere_get_node_ids(id);
    hits = loads.loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(id);
    times = loads.loadstrajectory_bodyheatfluxsphere_get_times(id);
    elements = loads.loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(id);
  }
  else
  {
    return false;
  }
  for (const auto& samples : elements)
  {
    for (std::size_t ring = 0; ring < samples.size(); ++ring)
    {
      for (int element : samples[ring])
      {
        if (!CoreDrawMeshGeometry::append(faces ? "face" : "element", element, false, CoreDrawEntityGeometry::color(static_cast<int>(ring)), geometry))
        {
          return false;
        }
      }
    }
  }
  for (std::size_t index = 0; index < nodes.size() && index < hits.size(); ++index)
  {
    if (hits[index].size() < 3)
    {
      continue;
    }
    const auto point = CubitInterface::get_nodal_coordinates(nodes[index]);
    if (!geometry.point({point[0], point[1], point[2]}, "red") || !geometry.line({point[0], point[1], point[2]}, {hits[index][0], hits[index][1], hits[index][2]}, "red"))
    {
      return false;
    }
  }
  if (!nodes.empty() && !times.empty() && times.front().size() >= 2 && times.back().size() >= 2)
  {
    const auto first = CubitInterface::get_nodal_coordinates(nodes.front());
    const auto last = CubitInterface::get_nodal_coordinates(nodes.back());
    geometry.label({first[0], first[1], first[2]}, "t_begin " + std::to_string(times.front()[0]), "yellow");
    geometry.label({last[0], last[1], last[2]}, "t_end " + std::to_string(times.back()[1]), "yellow");
  }
  output = std::move(geometry);
  return true;
}

bool buildFilm(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_load_film(id);
  if (!validRows(draw_data, 6))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!DrawingSymbols::arrow(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "indianred", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildRadiation(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_load_radiation(id);
  if (!validRows(draw_data, 6))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!DrawingSymbols::arrow(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "tomato", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildSurfaceTraction(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  //std::string log = "Surface Traction ID " + std::to_string(id) + "  drawn with size " + std::to_string(size) +"\n";
  //PRINT_INFO("%s", log.c_str());

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_load_surface_traction(id);
  if (!validRows(draw_data, 6))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!DrawingSymbols::arrow(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {-draw_data[i][3],-draw_data[i][4],-draw_data[i][5]}, true, "darkgreen", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildBodyHeatflux(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface drawing;
  const auto data = drawing.get_draw_data_for_load_bodyheatflux(id);
  if (data.size() != 2)
  {
    return false;
  }
  for (int block : data[0])
  {
    if (!CoreDrawMeshGeometry::append("block", block, false, "purple", geometry))
    {
      return false;
    }
  }
  for (int element : data[1])
  {
    if (!CoreDrawMeshGeometry::append("element", element, false, "purple", geometry))
    {
      return false;
    }
  }
  output = std::move(geometry);
  return true;
}

bool buildDisplacement(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_bc_displacement(id);
  if (!validRows(draw_data, 4))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!CalculiXDrawingAdapter::displacement(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, draw_data[i][3], "coral", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}

bool buildTemperature(int id, double size, DrawingGeometry& output)
{
  DrawingGeometry geometry;

  CalculiXDrawInterface ccx_draw_iface;

  std::vector<std::vector<double>> draw_data;
  draw_data = ccx_draw_iface.get_draw_data_for_bc_temperature(id);
  if (!validRows(draw_data, 4))
  {
    return false;
  }

  for (size_t i = 0; i < draw_data.size(); i++)
  {
    if (!CalculiXDrawingAdapter::displacement(geometry, {draw_data[i][0],draw_data[i][1],draw_data[i][2]}, draw_data[i][3], "red", size))
    {
      return false;
    }
  }

  output = std::move(geometry);
  return true;
}
}

CoreDrawEntityGeometry::CoreDrawEntityGeometry() = default;

bool CoreDrawEntityGeometry::build(DrawEntityKey entity, double symbolSize, DrawingGeometry& output, std::string& error)
{
  error.clear();
  if (entity.id <= 0 || !std::isfinite(symbolSize) || symbolSize <= 0)
  {
    error = "Invalid drawing entity or symbol size.";
    return false;
  }
  DrawingGeometry candidate;
  bool valid = false;
  try
  {
    switch (entity.feature)
    {
      case DrawFeature::Force: valid = buildForce(entity.id, symbolSize, candidate); break;
      case DrawFeature::Pressure: valid = buildPressure(entity.id, symbolSize, candidate); break;
      case DrawFeature::Heatflux: valid = buildHeatflux(entity.id, symbolSize, candidate); break;
      case DrawFeature::Gravity: valid = buildGravity(entity.id, symbolSize, candidate); break;
      case DrawFeature::Centrifugal: valid = buildCentrifugal(entity.id, symbolSize, candidate); break;
      case DrawFeature::Trajectory: valid = buildTrajectory(entity.id, symbolSize, candidate); break;
      case DrawFeature::Film: valid = buildFilm(entity.id, symbolSize, candidate); break;
      case DrawFeature::Radiation: valid = buildRadiation(entity.id, symbolSize, candidate); break;
      case DrawFeature::SurfaceTraction: valid = buildSurfaceTraction(entity.id, symbolSize, candidate); break;
      case DrawFeature::BodyHeatflux: valid = buildBodyHeatflux(entity.id, symbolSize, candidate); break;
      case DrawFeature::Displacement: valid = buildDisplacement(entity.id, symbolSize, candidate); break;
      case DrawFeature::Temperature: valid = buildTemperature(entity.id, symbolSize, candidate); break;

      default: error = "Only load and BC symbol geometry is supported."; return false;
    }
  }
  catch (const std::exception& exception)
  {
    error = std::string("Invalid stored drawing definition: ") + exception.what();
    return false;
  }
  if (!valid)
  {
    error = "Cannot build load or BC geometry for entity " + std::to_string(entity.id) + ".";
    return false;
  }
  output = std::move(candidate);
  return true;
}

std::string CoreDrawEntityGeometry::color(int color_id)
{
  std::vector<std::string> colors;
  colors.push_back("red");
  colors.push_back("orange");
  colors.push_back("yellow");
  colors.push_back("salmon");
  colors.push_back("coral");
  colors.push_back("pink");
  colors.push_back("purple");
  colors.push_back("paleturquoise");
  colors.push_back("lightsalmon");
  colors.push_back("springgreen");
  colors.push_back("slateblue");
  colors.push_back("sienna");
  colors.push_back("seagreen");
  colors.push_back("deepskyblue");
  colors.push_back("lightblue");
  colors.push_back("lightgreen");
  colors.push_back("khaki");
  colors.push_back("tomato");
  colors.push_back("lightskyblue");
  colors.push_back("turquoise");
  colors.push_back("greenyellow");
  colors.push_back("powderblue");
  colors.push_back("mediumturquoise");
  colors.push_back("skyblue");
  colors.push_back("lightcyan");
  colors.push_back("dodgerblue");
  colors.push_back("aquamarine");
  colors.push_back("lightgoldenrodyellow");
  colors.push_back("darkgreen");
  colors.push_back("lightcoral");
  colors.push_back("green");
  colors.push_back("magenta");
  colors.push_back("cyan");
  colors.push_back("blue");
  colors.push_back("white");
  colors.push_back("brown");
  colors.push_back("gold");

  if ((color_id>0)&&(color_id<38))
  {
    return colors[color_id];
  }
  return colors[0];
}
