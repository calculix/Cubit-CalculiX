#ifndef CORELOADSQUERY_HPP
#define CORELOADSQUERY_HPP

#include <string>
#include <vector>

class CoreLoadsForces;
class CoreLoadsPressures;
class CoreLoadsHeatfluxes;
class CoreLoadsGravity;
class CoreLoadsCentrifugal;
class CoreLoadsFilm;
class CoreLoadsRadiation;
class CoreLoadsSurfaceTraction;
class CoreLoadsTrajectory;
class CoreLoadsBodyHeatflux;

struct CoreLoadsQueryData
{
  CoreLoadsForces* forces;
  CoreLoadsPressures* pressures;
  CoreLoadsHeatfluxes* heatfluxes;
  CoreLoadsGravity* gravity;
  CoreLoadsCentrifugal* centrifugal;
  CoreLoadsTrajectory* trajectory;
  CoreLoadsFilm* film;
  CoreLoadsRadiation* radiation;
  CoreLoadsSurfaceTraction* traction;
  CoreLoadsBodyHeatflux* bodyheatflux;
};

class CoreLoadsQuery
{
public:
  static bool load_exists(const CoreLoadsQueryData& data, int load_type, int load_id);
  static std::vector<int> get_load_ids(const CoreLoadsQueryData& data, const std::string& parse_type);
  static std::vector<std::vector<std::string>> get_entities(const CoreLoadsQueryData& data, const std::string& entity, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsForces& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsPressures& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsHeatfluxes& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsGravity& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsCentrifugal& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsFilm& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsRadiation& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsSurfaceTraction& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsTrajectory& loads, int load_id);
  static std::vector<std::vector<std::string>> get_entities(CoreLoadsBodyHeatflux& loads, int load_id);
};

#endif
