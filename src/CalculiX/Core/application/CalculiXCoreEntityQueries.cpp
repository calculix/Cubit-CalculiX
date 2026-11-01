#include "Core/CalculiXCore.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/sections/CoreSectionsEntities.hpp"
#include "Core/interactions/CoreConstraintsEntities.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/bcs/CoreBCsEntities.hpp"
#include "Core/steps/CoreInitialConditionsEntities.hpp"
#include "Core/loads/CoreLoadsQuery.hpp"

CoreLoadsQueryData CalculiXCore::get_load_query_data() const
{
  return {
    loadsforces, loadspressures, loadsheatfluxes, loadsgravity,
    loadscentrifugal, loadstrajectory, loadsfilm, loadsradiation,
    loadssurfacetraction, loadsbodyheatflux
  };
}

bool CalculiXCore::check_bc_exists(int bc_id, int BCType)
{
  if (BCType == 4)
  {
    return bcsdisplacements->get_bcs_data_id_from_bc_id(bc_id) != -1;
  }

  if (BCType == 5)
  {
    return bcstemperatures->get_bcs_data_id_from_bc_id(bc_id) != -1;
  }

  if (BCType < 6 || BCType > 15)
  {
    return false;
  }

  return CoreLoadsQuery::load_exists(get_load_query_data(), BCType - 5, bc_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_entities(std::string entity, int id)
{
  if (entity == "block")
  {
    return cb->get_entities(id);
  }

  if (entity == "nodeset" || entity == "sideset")
  {
    return {{entity, std::to_string(id)}};
  }

  if (entity == "material")
  {
    return CoreSectionsEntities::get_material_entities(*sections, id);
  }

  if (entity == "section")
  {
    return CoreSectionsEntities::get_section_entities(*sections, id);
  }

  if (entity == "constraint")
  {
    return CoreConstraintsEntities::get_entities(*constraints, id);
  }

  if (entity == "contactpair")
  {
    return contactpairs->get_entities(id);
  }

  if (entity == "bcsdisplacement" || entity == "hbcsdisplacement")
  {
    return CoreBCsEntities::get_entities(*bcsdisplacements, id);
  }

  if (entity == "bcstemperature" || entity == "hbcstemperature")
  {
    return CoreBCsEntities::get_entities(*bcstemperatures, id);
  }

  if (entity == "initialcondition")
  {
    return CoreInitialConditionsEntities::get_entities(*initialconditions, *bcsdisplacements, *bcstemperatures, id);
  }

  return CoreLoadsQuery::get_entities(get_load_query_data(), entity, id);
}
