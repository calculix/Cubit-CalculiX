#ifndef COREBCSENTITIES_HPP
#define COREBCSENTITIES_HPP

#include <string>
#include <vector>

class CoreBCsDisplacements;
class CoreBCsTemperatures;

class CoreBCsEntities
{
public:
  static std::vector<std::vector<std::string>> get_entities(CoreBCsDisplacements& bcs, int bc_id);
  static std::vector<std::vector<std::string>> get_entities(CoreBCsTemperatures& bcs, int bc_id);
};

#endif
