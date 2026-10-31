#ifndef COREDRAWLOADDATA_HPP
#define COREDRAWLOADDATA_HPP

#include <vector>

class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsBodyHeatflux;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class MeshExportInterface;

class CoreDrawLoadData
{
public:
  static std::vector<std::vector<double>> get_draw_data_for_load_force(
    CoreLoadsForces& loads, MeshExportInterface& mesh, int id);

  static std::vector<std::vector<double>> get_draw_data_for_load_pressure(
    CoreLoadsPressures& loads, MeshExportInterface& mesh, int id);

  static std::vector<std::vector<double>> get_draw_data_for_load_heatflux(
    CoreLoadsHeatfluxes& loads, MeshExportInterface& mesh, int id);

  static std::vector<std::vector<double>> get_draw_data_for_load_film(CoreLoadsFilm& loads, int id);
  static std::vector<std::vector<double>> get_draw_data_for_load_radiation(CoreLoadsRadiation& loads, int id);
  static std::vector<std::vector<double>> get_draw_data_for_load_surface_traction(CoreLoadsSurfaceTraction& loads, int id);
  static std::vector<std::vector<int>> get_draw_data_for_load_bodyheatflux(CoreLoadsBodyHeatflux& loads, int id);
  static std::vector<std::vector<double>> get_draw_data_for_load_gravity(CoreLoadsGravity& loads, int id);
  static std::vector<std::vector<double>> get_draw_data_for_load_centrifugal(CoreLoadsCentrifugal& loads, int id);
};

#endif
