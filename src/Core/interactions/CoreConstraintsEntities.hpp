#ifndef CORECONSTRAINTSENTITIES_HPP
#define CORECONSTRAINTSENTITIES_HPP

#include <string>
#include <vector>

class CoreConstraints;

class CoreConstraintsEntities
{
public:
  static std::vector<std::vector<std::string>> get_entities(CoreConstraints& constraints, int constraint_id);
};

#endif
