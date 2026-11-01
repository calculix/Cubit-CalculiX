#include "Core/CalculiXCore.hpp"
#include "CoreLoadsForces.hpp"
#include "CoreLoadsPressures.hpp"
#include "CoreLoadsHeatfluxes.hpp"
#include "CoreLoadsGravity.hpp"
#include "CoreLoadsCentrifugal.hpp"
#include "CoreLoadsFilm.hpp"
#include "CoreLoadsRadiation.hpp"
#include "CoreLoadsSurfaceTraction.hpp"
#include "CoreLoadsBodyHeatflux.hpp"
#include "CoreLoadRemoval.hpp"
#include "CoreLoadsTreeData.hpp"

std::vector<int> CalculiXCore::get_loadsforces_ids()
{
  return loadsforces->get_load_ids();
}

bool CalculiXCore::create_loadsforces(std::vector<std::string> options, std::vector<double> options2)
{
  return loadsforces->create_load(options, options2);
}

bool CalculiXCore::modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return loadsforces->modify_load(force_id, options, options2, options_marker);
}

bool CalculiXCore::delete_loadsforces(int force_id)
{
  return CoreLoadRemoval::delete_load(*loadsforces, *steps, force_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsforces_tree_data()
{
  return CoreLoadsTreeData::get_loadsforces_tree_data(*loadsforces);
}

std::vector<int> CalculiXCore::get_loadspressures_ids()
{
  return loadspressures->get_load_ids();
}

bool CalculiXCore::create_loadspressures(std::vector<std::string> options, std::vector<double> options2)
{
  return loadspressures->create_load(options, options2);
}

bool CalculiXCore::modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return loadspressures->modify_load(pressure_id, options, options2, options_marker);
}

bool CalculiXCore::delete_loadspressures(int pressure_id)
{
  return CoreLoadRemoval::delete_load(*loadspressures, *steps, pressure_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadspressures_tree_data()
{
  return CoreLoadsTreeData::get_loadspressures_tree_data(*loadspressures);
}

std::vector<int> CalculiXCore::get_loadsheatfluxes_ids()
{
  return loadsheatfluxes->get_load_ids();
}

bool CalculiXCore::create_loadsheatfluxes(std::vector<std::string> options, std::vector<double> options2)
{
  return loadsheatfluxes->create_load(options, options2);
}

bool CalculiXCore::modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return loadsheatfluxes->modify_load(heatflux_id, options, options2, options_marker);
}

bool CalculiXCore::delete_loadsheatfluxes(int heatflux_id)
{
  return CoreLoadRemoval::delete_load(*loadsheatfluxes, *steps, heatflux_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsheatfluxes_tree_data()
{
  return CoreLoadsTreeData::get_loadsheatfluxes_tree_data(*loadsheatfluxes);
}

std::vector<int> CalculiXCore::get_loadsgravity_ids()
{
  return loadsgravity->get_load_ids();
}

bool CalculiXCore::create_loadsgravity(std::vector<std::string> options)
{
  return loadsgravity->create_load(options);
}

bool CalculiXCore::modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsgravity->modify_load(gravity_id, options, options_marker);
}

bool CalculiXCore::delete_loadsgravity(int gravity_id)
{
  return CoreLoadRemoval::delete_load(*loadsgravity, *steps, gravity_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsgravity_tree_data()
{
  return CoreLoadsTreeData::get_loadsgravity_tree_data(*loadsgravity);
}

std::vector<int> CalculiXCore::get_loadscentrifugal_ids()
{
  return loadscentrifugal->get_load_ids();
}

bool CalculiXCore::create_loadscentrifugal(std::vector<std::string> options)
{
  return loadscentrifugal->create_load(options);
}

bool CalculiXCore::modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadscentrifugal->modify_load(centrifugal_id, options, options_marker);
}

bool CalculiXCore::delete_loadscentrifugal(int centrifugal_id)
{
  return CoreLoadRemoval::delete_load(*loadscentrifugal, *steps, centrifugal_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadscentrifugal_tree_data()
{
  return CoreLoadsTreeData::get_loadscentrifugal_tree_data(*loadscentrifugal);
}

std::vector<int> CalculiXCore::get_loadsfilm_ids()
{
  return loadsfilm->get_load_ids();
}

bool CalculiXCore::create_loadsfilm(std::vector<std::string> options)
{
  return loadsfilm->create_load(options);
}

bool CalculiXCore::modify_loadsfilm(int film_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsfilm->modify_load(film_id, options, options_marker);
}

bool CalculiXCore::delete_loadsfilm(int film_id)
{
  return CoreLoadRemoval::delete_load(*loadsfilm, *steps, film_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsfilm_tree_data()
{
  return CoreLoadsTreeData::get_loadsfilm_tree_data(*loadsfilm);
}

std::vector<int> CalculiXCore::get_loadsradiation_ids()
{
  return loadsradiation->get_load_ids();
}

bool CalculiXCore::create_loadsradiation(std::vector<std::string> options)
{
  return loadsradiation->create_load(options);
}

bool CalculiXCore::modify_loadsradiation(int radiation_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsradiation->modify_load(radiation_id, options, options_marker);
}

bool CalculiXCore::delete_loadsradiation(int radiation_id)
{
  return CoreLoadRemoval::delete_load(*loadsradiation, *steps, radiation_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsradiation_tree_data()
{
  return CoreLoadsTreeData::get_loadsradiation_tree_data(*loadsradiation);
}

std::vector<int> CalculiXCore::get_loadssurfacetraction_ids()
{
  return loadssurfacetraction->get_load_ids();
}

bool CalculiXCore::create_loadssurfacetraction(std::vector<std::string> options, std::vector<double> options2)
{
  return loadssurfacetraction->create_load(options, options2);
}

bool CalculiXCore::modify_loadssurfacetraction(int surfacetraction_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return loadssurfacetraction->modify_load(surfacetraction_id, options, options2, options_marker);
}

bool CalculiXCore::delete_loadssurfacetraction(int surfacetraction_id)
{
  return CoreLoadRemoval::delete_load(*loadssurfacetraction, *steps, surfacetraction_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadssurfacetraction_tree_data()
{
  return CoreLoadsTreeData::get_loadssurfacetraction_tree_data(*loadssurfacetraction);
}

std::vector<int> CalculiXCore::get_loadsbodyheatflux_ids()
{
  return loadsbodyheatflux->get_load_ids();
}

bool CalculiXCore::create_loadsbodyheatflux(std::vector<std::string> options, std::vector<int> options2)
{
  return loadsbodyheatflux->create_load(options, options2);
}

bool CalculiXCore::modify_loadsbodyheatflux(int bodyheatflux_id, std::vector<std::string> options, std::vector<int> options2, std::vector<int> options_marker)
{
  return loadsbodyheatflux->modify_load(bodyheatflux_id, options, options2, options_marker);
}

bool CalculiXCore::delete_loadsbodyheatflux(int bodyheatflux_id)
{
  return CoreLoadRemoval::delete_load(*loadsbodyheatflux, *steps, bodyheatflux_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsbodyheatflux_tree_data()
{
  return CoreLoadsTreeData::get_loadsbodyheatflux_tree_data(*loadsbodyheatflux);
}
