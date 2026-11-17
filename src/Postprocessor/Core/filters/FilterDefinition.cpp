#include "FilterDefinition.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <algorithm>
namespace Postprocessor
{
  bool valid_filter(const FilterDefinition& filter,std::string& error)
  {
    if (filter.kind != FilterKind::Threshold && !filter.coordinate_unit.empty())
    {
      double ignored = 0.0;
      if (!framework_units().to_si(0.0,filter.coordinate_unit,SimUnits::Length,SimUnits::UnitRole::Linear,ignored,error))
      {
        return false;
      }
    }
    auto plane = filter.plane;
    if (filter.name.empty() || filter.name.size() > 256 || filter.id.empty() || filter.input.owner != OutputOwner::Source ||
        (filter.name.find('\"') != std::string::npos && filter.name.find("'") != std::string::npos) ||
        filter.input.port != 0 || filter.input.object_id.empty() || filter.revision == 0 ||
        (filter.kind != FilterKind::Clip && filter.kind != FilterKind::Section && filter.kind != FilterKind::Threshold) ||
        (filter.keep != ClipSide::Positive && filter.keep != ClipSide::Negative) ||
        std::any_of(filter.name.begin(),filter.name.end(),[](unsigned char c)
        {
          return c < 32 || c == 127;
        }) || (filter.kind != FilterKind::Threshold && !normalize_plane(plane,error)))
    {
      if (error.empty())
      {
        error = "Invalid filter name, source, kind or plane.";
      }
      return false;
    }
    if (filter.kind == FilterKind::Threshold)
    {
      return valid_threshold(filter.threshold,error);
    }
    return true;
  }
}
