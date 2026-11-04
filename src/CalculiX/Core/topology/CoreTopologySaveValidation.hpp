#ifndef CORETOPOLOGYSAVEVALIDATION_HPP
#define CORETOPOLOGYSAVEVALIDATION_HPP

#include "CoreTopologyStructs.hpp"

/** @brief Validate stored numeric history without querying the current Cubit mesh.
 * Prepared and interrupted records are permitted. A selected iteration must have
 * a design, physical job and objective; proposal availability is independent.
 * No controller state or sparse operators are restored from these records.
 */
class CoreTopologySaveValidation
{
public:
  static bool validate_case(const TopologyCase& configuration);
  static bool validate_factors(const TopologyCase& configuration, const std::map<int, int>& assignments, const std::map<int, std::map<int, double>>& factors);
};

#endif
