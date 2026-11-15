#ifndef CORERESULTSUNITS_HPP
#define CORERESULTSUNITS_HPP
#include "Framework/Results/VtkResultsData.hpp"
class CoreResultsUnits {
public:
  static bool annotate(SimResults::VtkResultsData& result, std::string& error);
};
#endif
