#ifndef CORERESULTSUNITS_HPP
#define CORERESULTSUNITS_HPP
#include "Framework/Results/VtkResultsData.hpp"
#include <map>
class CoreResultsUnits {
public:
  // Pre-publication operation: writes component/table metadata and its VTK
  // serialization, never scales numeric result arrays. Source units describe the
  // solved data, independently of current display preferences. Unknown meanings
  // or missing dimensional source units remain explicit unresolved descriptors.
  static bool annotate(SimResults::VtkResultsData& result, std::string& error, const std::map<int, std::string>& step_axis_quantities = {});
};
#endif
