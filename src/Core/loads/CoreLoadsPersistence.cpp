#include "Core/loads/CoreLoadsPersistence.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "HDF5Tool.hpp"

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsForces& forces)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Forces", forces.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Forces", forces.time_delay_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Forces", forces.name_data);
  file.read_dataset_double_rank_2("force_data", "Cubit-CalculiX/Loads/Forces", forces.force_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsForces& forces)
{
  file.createGroup("Cubit-CalculiX/Loads/Forces");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Forces", forces.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Forces", forces.time_delay_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Forces", forces.name_data);
  file.write_dataset_double_rank_2("force_data", "Cubit-CalculiX/Loads/Forces", forces.force_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsPressures& pressures)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Pressures", pressures.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Pressures", pressures.time_delay_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Pressures", pressures.name_data);
  file.read_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Pressures", pressures.magnitude_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsPressures& pressures)
{
  file.createGroup("Cubit-CalculiX/Loads/Pressures");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Pressures", pressures.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Pressures", pressures.time_delay_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Pressures", pressures.name_data);
  file.write_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Pressures", pressures.magnitude_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsHeatfluxes& heatfluxes)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.time_delay_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.name_data);
  file.read_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.magnitude_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsHeatfluxes& heatfluxes)
{
  file.createGroup("Cubit-CalculiX/Loads/HeatFluxes");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.time_delay_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.name_data);
  file.write_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/HeatFluxes", heatfluxes.magnitude_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsGravity& gravity)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Gravity", gravity.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Gravity", gravity.time_delay_data);
  file.read_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Gravity", gravity.direction_data);
  file.read_dataset_string_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Gravity", gravity.magnitude_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Gravity", gravity.name_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsGravity& gravity)
{
  file.createGroup("Cubit-CalculiX/Loads/Gravity");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Gravity", gravity.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Gravity", gravity.time_delay_data);
  file.write_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Gravity", gravity.direction_data);
  file.write_dataset_string_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Gravity", gravity.magnitude_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Gravity", gravity.name_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsCentrifugal& centrifugal)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.time_delay_data);
  file.read_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.direction_data);
  file.read_dataset_string_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.magnitude_data);
  file.read_dataset_string_rank_2("coordinate_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.coordinate_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.name_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsCentrifugal& centrifugal)
{
  file.createGroup("Cubit-CalculiX/Loads/Centrifugal");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.time_delay_data);
  file.write_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.direction_data);
  file.write_dataset_string_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.magnitude_data);
  file.write_dataset_string_rank_2("coordinate_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.coordinate_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Centrifugal", centrifugal.name_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsTrajectory& trajectory)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Trajectory", trajectory.loads_data);

  auto& heatflux = *trajectory.heatflux;
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.loads_data);
  file.read_dataset_int_rank_2("fire_ray_surface_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.fire_ray_surface_data);
  file.read_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.direction_data);
  file.read_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.magnitude_data);
  file.read_dataset_double_rank_2("radius_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.radius_data);
  file.read_dataset_string_rank_2("time_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.time_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.name_data);

  auto& sphere = *trajectory.bodyheatfluxsphere;
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.loads_data);
  file.read_dataset_int_rank_2("fire_ray_surface_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.fire_ray_surface_data);
  file.read_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.direction_data);
  file.read_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.magnitude_data);
  file.read_dataset_double_rank_2("radius_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.radius_data);
  file.read_dataset_double_rank_2("depth_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.depth_data);
  file.read_dataset_string_rank_2("time_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.time_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.name_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsTrajectory& trajectory)
{
  file.createGroup("Cubit-CalculiX/Loads/Trajectory");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Trajectory", trajectory.loads_data);

  const auto& heatflux = *trajectory.heatflux;
  file.createGroup("Cubit-CalculiX/Loads/Trajectory/Heatflux");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.loads_data);
  file.write_dataset_int_rank_2("fire_ray_surface_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.fire_ray_surface_data);
  file.write_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.direction_data);
  file.write_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.magnitude_data);
  file.write_dataset_double_rank_2("radius_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.radius_data);
  file.write_dataset_string_rank_2("time_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.time_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Trajectory/Heatflux", heatflux.name_data);

  const auto& sphere = *trajectory.bodyheatfluxsphere;
  file.createGroup("Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.loads_data);
  file.write_dataset_int_rank_2("fire_ray_surface_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.fire_ray_surface_data);
  file.write_dataset_string_rank_2("direction_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.direction_data);
  file.write_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.magnitude_data);
  file.write_dataset_double_rank_2("radius_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.radius_data);
  file.write_dataset_double_rank_2("depth_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.depth_data);
  file.write_dataset_string_rank_2("time_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.time_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", sphere.name_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsFilm& film)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Film", film.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Film", film.time_delay_data);
  file.read_dataset_string_rank_2("temperature_data", "Cubit-CalculiX/Loads/Film", film.temperature_data);
  file.read_dataset_string_rank_2("coefficient_data", "Cubit-CalculiX/Loads/Film", film.coefficient_data);
  file.read_dataset_string_rank_2("film_time_delay_data", "Cubit-CalculiX/Loads/Film", film.film_time_delay_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Film", film.name_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsFilm& film)
{
  file.createGroup("Cubit-CalculiX/Loads/Film");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Film", film.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Film", film.time_delay_data);
  file.write_dataset_string_rank_2("temperature_data", "Cubit-CalculiX/Loads/Film", film.temperature_data);
  file.write_dataset_string_rank_2("coefficient_data", "Cubit-CalculiX/Loads/Film", film.coefficient_data);
  file.write_dataset_string_rank_2("film_time_delay_data", "Cubit-CalculiX/Loads/Film", film.film_time_delay_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Film", film.name_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsRadiation& radiation)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Radiation", radiation.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Radiation", radiation.time_delay_data);
  file.read_dataset_string_rank_2("temperature_data", "Cubit-CalculiX/Loads/Radiation", radiation.temperature_data);
  file.read_dataset_string_rank_2("emissivity_data", "Cubit-CalculiX/Loads/Radiation", radiation.emissivity_data);
  file.read_dataset_string_rank_2("radiation_time_delay_data", "Cubit-CalculiX/Loads/Radiation", radiation.radiation_time_delay_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Radiation", radiation.name_data);
  file.read_dataset_string_rank_2("cavity_data", "Cubit-CalculiX/Loads/Radiation", radiation.cavity_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsRadiation& radiation)
{
  file.createGroup("Cubit-CalculiX/Loads/Radiation");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/Radiation", radiation.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/Radiation", radiation.time_delay_data);
  file.write_dataset_string_rank_2("temperature_data", "Cubit-CalculiX/Loads/Radiation", radiation.temperature_data);
  file.write_dataset_string_rank_2("emissivity_data", "Cubit-CalculiX/Loads/Radiation", radiation.emissivity_data);
  file.write_dataset_string_rank_2("radiation_time_delay_data", "Cubit-CalculiX/Loads/Radiation", radiation.radiation_time_delay_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/Radiation", radiation.name_data);
  file.write_dataset_string_rank_2("cavity_data", "Cubit-CalculiX/Loads/Radiation", radiation.cavity_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsSurfaceTraction& traction)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.time_delay_data);
  file.read_dataset_double_rank_2("force_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.force_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.name_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsSurfaceTraction& traction)
{
  file.createGroup("Cubit-CalculiX/Loads/SurfaceTraction");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.time_delay_data);
  file.write_dataset_double_rank_2("force_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.force_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/SurfaceTraction", traction.name_data);
}

void CoreLoadsPersistence::read(HDF5Tool& file, CoreLoadsBodyHeatflux& bodyheatflux)
{
  file.read_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.loads_data);
  file.read_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.time_delay_data);
  file.read_dataset_int_rank_2("element_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.element_data);
  file.read_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.magnitude_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.name_data);
}

void CoreLoadsPersistence::write(HDF5Tool& file, const CoreLoadsBodyHeatflux& bodyheatflux)
{
  file.createGroup("Cubit-CalculiX/Loads/BodyHeatflux");
  file.write_dataset_int_rank_2("loads_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.loads_data);
  file.write_dataset_string_rank_2("time_delay_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.time_delay_data);
  file.write_dataset_int_rank_2("element_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.element_data);
  file.write_dataset_double_rank_2("magnitude_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.magnitude_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/Loads/BodyHeatflux", bodyheatflux.name_data);
}
