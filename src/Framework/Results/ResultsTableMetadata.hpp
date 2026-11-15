#ifndef CUBITSIM_RESULTSTABLEMETADATA_HPP
#define CUBITSIM_RESULTSTABLEMETADATA_HPP
#include "Framework/Core/units/QuantityUnit.hpp"
#include <string>
#include <vector>
namespace SimResults {
class ResultsTableMetadata {
public:
  ResultsTableMetadata();
  std::string name;
  std::vector<SimUnits::QuantityUnit> columns;
};
}
#endif
