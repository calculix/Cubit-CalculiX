#include "Core/CalculiXCore.hpp"
#include "CoreDrawLoadData.hpp"
#include "CoreDrawBCData.hpp"
#include "CoreDrawOrientationData.hpp"
#include "CoreDrawConstraintData.hpp"

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_force(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_force(*loadsforces, *me_iface, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_pressure(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_pressure(*loadspressures, *me_iface, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_heatflux(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_heatflux(*loadsheatfluxes, *me_iface, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_gravity(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_gravity(*loadsgravity, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_centrifugal(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_centrifugal(*loadscentrifugal, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_film(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_film(*loadsfilm, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_radiation(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_radiation(*loadsradiation, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_surface_traction(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_surface_traction(*loadssurfacetraction, id);
}

std::vector<std::vector<int>> CalculiXCore::get_draw_data_for_load_bodyheatflux(int id)
{
  return CoreDrawLoadData::get_draw_data_for_load_bodyheatflux(*loadsbodyheatflux, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_bc_displacement(int id)
{
  return CoreDrawBCData::get_draw_data_for_bc_displacement(*bcsdisplacements, *me_iface, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_bc_temperature(int id)
{
  return CoreDrawBCData::get_draw_data_for_bc_temperature(*bcstemperatures, *me_iface, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_orientation(int id)
{
  return CoreDrawOrientationData::get_draw_data_for_orientation(*orientations, *sections, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_equation(int id)
{
  return CoreDrawConstraintData::get_draw_data_for_equation(*constraints, id);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_equationgroup(int id)
{
  return CoreDrawConstraintData::get_draw_data_for_equationgroup(*constraints, id);
}
