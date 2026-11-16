#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "FilterDefinition.hpp"
#include <algorithm>
#include <cmath>
namespace Postprocessor
{
  bool normalize_plane(PlaneDefinition& plane,std::string& error)
  {
    error.clear();
    const auto finite = [](const auto& values)
    {
      return std::all_of(values.begin(),values.end(),[](double x)
      {
        return std::isfinite(x);
      });
    };
    if (!finite(plane.origin) || !finite(plane.normal))
    {
      error = "Plane origin and normal must be finite.";
      return false;
    }
    const double scale = std::max({std::abs(plane.normal[0]),std::abs(plane.normal[1]),std::abs(plane.normal[2])});
    if (scale == 0)
    {
      error = "Plane normal must be nonzero.";
      return false;
    }
    auto candidate = plane;
    for (auto& x : candidate.normal)
    {
      x /= scale;
    }
    const double length = std::hypot(candidate.normal[0],candidate.normal[1],candidate.normal[2]);
    for (auto& x : candidate.normal)
    {
      x /= length;
    }
    plane = candidate;
    return true;
  }
  double signed_distance(const PlaneDefinition& plane,const std::array<double,3>& point)
  {
    return plane.normal[0]*(point[0]-plane.origin[0])+plane.normal[1]*(point[1]-plane.origin[1])+plane.normal[2]*(point[2]-plane.origin[2]);
  }
  bool valid_filter(const FilterDefinition& filter,std::string& error)
  {
    if (!filter.coordinate_unit.empty())
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
        (filter.kind != FilterKind::Clip && filter.kind != FilterKind::Section) ||
        (filter.keep != ClipSide::Positive && filter.keep != ClipSide::Negative) ||
        std::any_of(filter.name.begin(),filter.name.end(),[](unsigned char c)
        {
          return c < 32 || c == 127;
        }) || !normalize_plane(plane,error))
    {
      if (error.empty())
      {
        error = "Invalid filter name, source, kind or plane.";
      }
      return false;
    }
    return true;
  }
}
