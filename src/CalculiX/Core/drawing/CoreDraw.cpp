#include "CoreDrawEntityGeometry.hpp"
#include "Core/drawing/CalculiXDrawingAdapter.hpp"
#include "Framework/Drawing/DrawingSymbols.hpp"
#include "CoreDrawMeshGeometry.hpp"
#include <QScopedValueRollback>
#include "Core/drawing/CoreDraw.hpp"
#include <string>
#include <vector>
#include <array>
#include <QString>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "TopologyAnimation.hpp"
#include "CubitColorTable.hpp"
#include "CubitVector.hpp"
#include "GCubitModelSet.hpp"
#include "GGenericModel.hpp"
#include "GView.hpp"
#include "GWindowFactory.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <sstream>
#include <cmath>
#include <set>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QTemporaryDir>
#include <QSaveFile>

CoreDraw::CoreDraw()
{}

CoreDraw::~CoreDraw()
{
  shutdownDrawing();
}


bool CoreDraw::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    resetToDefaults();
    is_initialized = true;
    return true;
  }
}

std::vector<double> CoreDraw::rotate(std::vector<double> coord, std::vector<double> vec_a, std::vector<double> vec_b)
{
  return DrawingSymbols::rotate(coord, vec_a, vec_b);
}

std::vector<double> CoreDraw::rotate_about_axis(std::vector<double> coord, std::vector<double> axis, double angle)
{
  return DrawingSymbols::rotate_about_axis(coord, axis, angle);
}

std::vector<double> CoreDraw::unit_vector(std::vector<double> input)
{
  return DrawingSymbols::unit_vector(input);
}


double CoreDraw::magnitude(std::vector<double> input)
{
  return DrawingSymbols::magnitude(input);
}

std::vector<double> CoreDraw::cross_product(std::vector<double> vec_a, std::vector<double> vec_b)
{
  return DrawingSymbols::cross_product(vec_a, vec_b);
}

std::vector<std::vector<double>> CoreDraw::cross_product_matrix(std::vector<double> vec)
{
  return DrawingSymbols::cross_product_matrix(vec);
}

double CoreDraw::inner_product(std::vector<double> vec_a, std::vector<double> vec_b)
{
  return DrawingSymbols::inner_product(vec_a, vec_b);
}

std::vector<double> CoreDraw::mult_vector_scalar(std::vector<double> vec, double scalar)
{
  return DrawingSymbols::mult_vector_scalar(vec, scalar);
}

std::vector<std::vector<double>> CoreDraw::mult_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b)
{
  return DrawingSymbols::mult_matrix(mat_a, mat_b);
}


std::vector<std::vector<double>> CoreDraw::mult_matrix_scalar(std::vector<std::vector<double>> mat_a, double scalar)
{
  return DrawingSymbols::mult_matrix_scalar(mat_a, scalar);
}

std::vector<std::vector<double>> CoreDraw::add_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b)
{
  return DrawingSymbols::add_matrix(mat_a, mat_b);
}

std::vector<double> CoreDraw::mult_matrix_vector(std::vector<std::vector<double>> mat, std::vector<double> vec)
{
  return DrawingSymbols::mult_matrix_vector(mat, vec);
}

bool CoreDraw::draw_arrow(std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size)
{
  if (currentGeometry == nullptr)
  {
    return false;
  }
  return DrawingSymbols::arrow(*currentGeometry, start_point, direction, from_start_point, color, size);
}

bool CoreDraw::draw_arrow_flat(std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size)
{
  if (currentGeometry == nullptr)
  {
    return false;
  }
  return DrawingSymbols::flatArrow(*currentGeometry, start_point, direction, from_start_point, color, size);
}

bool CoreDraw::draw_dof(std::vector<double> coord, int dof, std::string color, double size)
{
  if (currentGeometry == nullptr)
  {
    return false;
  }
  return CalculiXDrawingAdapter::displacement(*currentGeometry, coord, dof, color, size);
}

bool CoreDraw::draw_equation_node(std::vector<double> coord, int dof, std::string color, double size)
{
  if (currentGeometry == nullptr)
  {
    return false;
  }
  return CalculiXDrawingAdapter::equationNode(*currentGeometry, coord, dof, color, size);
}

bool CoreDraw::draw_label(std::vector<double> coord, std::string color, double size)
{
    //locate location 0 0 0 "test"
    return true;
}

bool CoreDraw::draw_load_force(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Force, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Force, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_pressure(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Pressure, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Pressure, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_heatflux(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Heatflux, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Heatflux, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_gravity(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Gravity, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Gravity, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_centrifugal(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Centrifugal, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Centrifugal, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_trajectory(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Trajectory, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Trajectory, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_film(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Film, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Film, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_radiation(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Radiation, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Radiation, id}, std::move(geometry), size);
}

bool CoreDraw::draw_load_surface_traction(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::SurfaceTraction, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::SurfaceTraction, id}, std::move(geometry), size);
}


bool CoreDraw::draw_load_bodyheatflux(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::BodyHeatflux, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::BodyHeatflux, id}, std::move(geometry), size);
}

bool CoreDraw::draw_bc_displacement(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Displacement, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Displacement, id}, std::move(geometry), size);
}

bool CoreDraw::draw_bc_temperature(int id, double size)
{
  DrawingGeometry geometry;
  std::string error;
  if (!CoreDrawEntityGeometry::build({DrawFeature::Temperature, id}, size, geometry, error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return storeGeometry({DrawFeature::Temperature, id}, std::move(geometry), size);
}

bool CoreDraw::draw_orientation(int id, double size)
{
  DrawingGeometry geometry;
  const QScopedValueRollback<DrawingGeometry*> targetGuard(currentGeometry, &geometry);

  CalculiXDrawInterface ccx_draw_iface;

    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_draw_iface.get_draw_data_for_orientation(id);

    for (size_t i = 0; i + 3 < draw_data.size(); i++)
    {
        // center point coordinates
        std::vector<double> center;
        center = draw_data[i+1];
        // axis
        std::vector<double> x_hat;
        std::vector<double> y_hat;
        std::vector<double> z_hat;

        //check if rectangular or cylindrical system
        if (draw_data[i][0]==1.0)
        {
            x_hat = draw_data[i+2];
            z_hat = this->cross_product(x_hat,draw_data[i+3]);
            y_hat = this->cross_product(x_hat,z_hat);

            //rotate about local axis
            if (draw_data[i][1]==1.0)
            {
                y_hat = this->rotate_about_axis(y_hat,x_hat,draw_data[i][2]);
                z_hat = this->rotate_about_axis(z_hat,x_hat,draw_data[i][2]);
            }else if (draw_data[i][1]==2.0)
            {
                x_hat = this->rotate_about_axis(x_hat,y_hat,-draw_data[i][2]);
                z_hat = this->rotate_about_axis(z_hat,y_hat,-draw_data[i][2]);
            }else if (draw_data[i][1]==3.0)
            {
                x_hat = this->rotate_about_axis(x_hat,z_hat,draw_data[i][2]);
                y_hat = this->rotate_about_axis(y_hat,z_hat,-draw_data[i][2]);
            }
            //inverse signum
            for (size_t ii = 0; ii < 3; ii++)
            {
                x_hat[ii] = -x_hat[ii];
                y_hat[ii] = -y_hat[ii];
                z_hat[ii] = -z_hat[ii];
            }

            if (!draw_arrow(center, x_hat, true, "green", size))
            {
              return false;
            }
            if (!draw_arrow(center, y_hat, true, "yellow", size))
            {
              return false;
            }
            if (!draw_arrow(center, z_hat, true, "red", size))
            {
              return false;
            }
        }else if (draw_data[i][0]==2.0)
        {
            for (size_t ii = 0; ii < 3; ii++)
            {
                z_hat.push_back(draw_data[i+2][ii]-draw_data[i+3][ii]);
            }
            if (!draw_arrow_flat(center, z_hat, true, "green", size))
            {
              return false;
            }
        }
        i+=3;
    }

    return storeGeometry({DrawFeature::Orientation, id}, std::move(geometry), size);
}

bool CoreDraw::draw_equation(int id, double size)
{
  DrawingGeometry geometry;
  const QScopedValueRollback<DrawingGeometry*> targetGuard(currentGeometry, &geometry);

  CalculiXDrawInterface drawing;
  const auto data = drawing.get_draw_data_for_equation(id);
  for (const auto& row : data)
  {
    if (row.size() < 4 || !draw_equation_node({row[0], row[1], row[2]}, static_cast<int>(row[3]), "", size))
    {
      return false;
    }
  }
  for (std::size_t index = 1; index < data.size(); ++index)
  {
    if (!graphics_line({data[0][0], data[0][1], data[0][2]}, {data[index][0], data[index][1], data[index][2]}, "yellow"))
    {
      return false;
    }
  }
  return storeGeometry({DrawFeature::Equation, id}, std::move(geometry), size);
}

bool CoreDraw::draw_equationgroup(int id, double size)
{
  DrawingGeometry geometry;
  const QScopedValueRollback<DrawingGeometry*> targetGuard(currentGeometry, &geometry);

  CalculiXDrawInterface drawing;
  const auto data = drawing.get_draw_data_for_equationgroup(id);
  if (data.size() % 2 != 0)
  {
    return false;
  }
  for (const auto& row : data)
  {
    if (row.size() < 3 || !graphics_point({row[0], row[1], row[2]}, "yellow"))
    {
      return false;
    }
  }
  for (std::size_t index = 0; index + 1 < data.size(); index += 2)
  {
    if (!graphics_line({data[index][0], data[index][1], data[index][2]}, {data[index + 1][0], data[index + 1][1], data[index + 1][2]}, "yellow"))
    {
      return false;
    }
  }
  return storeGeometry({DrawFeature::EquationGroup, id}, std::move(geometry), size);
}

bool CoreDraw::draw_loads(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;
    std::vector<int> tmp_bc_ids;

    tmp_load_ids = ccx_loads_iface.get_loadsforces_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_force(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadspressures_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_pressure(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadsheatfluxes_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_heatflux(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadsgravity_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_gravity(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadscentrifugal_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_centrifugal(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadstrajectory_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_trajectory(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadsfilm_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_film(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadsradiation_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_radiation(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadssurfacetraction_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_surface_traction(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    tmp_load_ids = ccx_loads_iface.get_loadsbodyheatflux_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_bodyheatflux(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_bcs(double size)
{
  CalculiXBCsInterface ccx_bcs_iface;

    std::vector<int> tmp_bc_ids;

    tmp_bc_ids = ccx_bcs_iface.get_bcsdisplacements_ids();
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        if (!draw_bc_displacement(tmp_bc_ids[i], size))
        {
          return false;
        }
    }

    tmp_bc_ids = ccx_bcs_iface.get_bcstemperatures_ids();
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        if (!draw_bc_temperature(tmp_bc_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_orientations(double size)
{
  CalculiXSectionsInterface ccx_sections_iface;

    std::vector<int> tmp_ids;

    tmp_ids = ccx_sections_iface.get_orientations_ids();
    for (size_t i = 0; i < tmp_ids.size(); i++)
    {
        draw_orientation(tmp_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_equations(double size)
{
  CalculiXInteractionsInterface ccx_interactions_iface;

    std::vector<int> tmp_ids;

    tmp_ids = ccx_interactions_iface.get_equation_ids();
    for (size_t i = 0; i < tmp_ids.size(); i++)
    {
        draw_equation(tmp_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_equationgroups(double size)
{
  CalculiXInteractionsInterface ccx_interactions_iface;

    std::vector<int> tmp_ids;

    tmp_ids = ccx_interactions_iface.get_equationgroup_ids();
    for (size_t i = 0; i < tmp_ids.size(); i++)
    {
        draw_equationgroup(tmp_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_all(double size)
{
  CalculiXApplicationInterface ccx_application_iface;

    if (!this->draw_bcs(size))
    {
      return false;
    }
    if (!this->draw_loads(size))
    {
      return false;
    }
    if (!this->draw_orientations(size))
    {
      return false;
    }
    if (!this->draw_equations(size))
    {
      return false;
    }
    if (!this->draw_equationgroups(size))
    {
      return false;
    }

    std::string log = "Loads, BCs, Orientations and Equations drawn with size " + ccx_application_iface.to_string_scientific(size) +"\n";
    PRINT_INFO("%s", log.c_str());

    return true;
}

bool CoreDraw::draw_load_forces(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadsforces_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_force(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_pressures(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadspressures_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_pressure(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_heatfluxes(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadsheatfluxes_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_heatflux(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_gravities(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadsgravity_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_gravity(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_centrifugals(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadscentrifugal_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_centrifugal(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_trajectories(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadstrajectory_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_trajectory(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_films(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadsfilm_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_film(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_radiations(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadsradiation_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_radiation(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_surface_tractions(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadssurfacetraction_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_surface_traction(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_load_bodyheatfluxes(double size)
{
  CalculiXLoadsInterface ccx_loads_iface;

    std::vector<int> tmp_load_ids;

    tmp_load_ids = ccx_loads_iface.get_loadsbodyheatflux_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        if (!draw_load_bodyheatflux(tmp_load_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_bc_displacements(double size)
{
  CalculiXBCsInterface ccx_bcs_iface;

    std::vector<int> tmp_bc_ids;

    tmp_bc_ids = ccx_bcs_iface.get_bcsdisplacements_ids();
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        if (!draw_bc_displacement(tmp_bc_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::draw_bc_temperatures(double size)
{
  CalculiXBCsInterface ccx_bcs_iface;

    std::vector<int> tmp_bc_ids;

    tmp_bc_ids = ccx_bcs_iface.get_bcstemperatures_ids();
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        if (!draw_bc_temperature(tmp_bc_ids[i], size))
        {
          return false;
        }
    }

    return true;
}

bool CoreDraw::animate_topology(int topology_id, double threshold, int fps, std::string save_filepath, bool overwrite)
{
  if (!is_initialized)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }
  if (!topology_animation)
  {
    topology_animation = std::make_unique<TopologyAnimation>();
  }
  return topology_animation->start(topology_id, threshold, fps, QString::fromStdString(save_filepath), overwrite);
}

bool CoreDraw::topology_animation_active() const
{
  return topology_animation && topology_animation->active();
}

void CoreDraw::cancel_topology_animation()
{
  if (topology_animation)
  {
    topology_animation->cancel();
  }
}

void CoreDraw::shutdown_topology_animation()
{
  topology_animation.reset(); // Cancels and joins the file-only worker before unloading.
}

std::string CoreDraw::get_color(int color_id)
{
  return CoreDrawEntityGeometry::color(color_id);
}


bool CoreDraw::graphics_point(const std::vector<double>& coord, const std::string& color)
{
  return currentGeometry != nullptr && currentGeometry->point(coord, color);
}

bool CoreDraw::graphics_line(const std::vector<double>& start, const std::vector<double>& end, const std::string& color)
{
  return currentGeometry != nullptr && currentGeometry->line(start, end, color);
}

bool CoreDraw::graphics_polygon(const std::vector<std::vector<double>>& coords, const std::string& color)
{
  return currentGeometry != nullptr && currentGeometry->polygon(coords, color);
}

bool CoreDraw::graphics_label(const std::vector<double>& coord, const std::string& text, const std::string& color)
{
  return currentGeometry != nullptr && currentGeometry->label(coord, text, color);
}

bool CoreDraw::graphics_flush()
{
  std::string error;
  if (!scene.synchronize(error))
  {
    PRINT_ERROR("%s\n", error.c_str());
    return false;
  }
  return true;
}

bool CoreDraw::graphics_clear()
{
  scene.clear(DrawGroup::Topology);
  scene.state.groups[DrawGroup::Topology].enabled = false;
  activeTopologyId = -1;
  return graphics_flush();
}
