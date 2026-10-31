#include "CoreBCsEntities.hpp"
#include "CoreBCsDisplacements.hpp"
#include "CoreBCsTemperatures.hpp"

namespace
{
  template<typename BCType>
  std::vector<std::vector<std::string>> get_nodeset_entities(BCType& bcs, int bc_id)
  {
    const int index = bcs.get_bcs_data_id_from_bc_id(bc_id);

    if (index == -1)
    {
      return {};
    }

    return {{"nodeset", std::to_string(bcs.bcs_data[index][4])}};
  }
}

std::vector<std::vector<std::string>> CoreBCsEntities::get_entities(CoreBCsDisplacements& bcs, int bc_id)
{
  return get_nodeset_entities(bcs, bc_id);
}

std::vector<std::vector<std::string>> CoreBCsEntities::get_entities(CoreBCsTemperatures& bcs, int bc_id)
{
  return get_nodeset_entities(bcs, bc_id);
}
