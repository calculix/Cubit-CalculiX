#pragma once
#include "../representations/Representation.hpp"
#include <cstdint>
namespace Postprocessor
{
  enum class FilterKind { Clip, Section };
  enum class ClipSide { Positive, Negative };
  namespace FilterDefaults
  {
    inline constexpr std::array<double,3> origin = {0,0,0};
    inline constexpr std::array<double,3> normal = {1,0,0};
    inline constexpr std::size_t maximum_filters = 128;
    inline constexpr std::array<double,3> plane_color = {0.95,0.55,0.05};
    inline constexpr double plane_line_width = 1.5;
    inline constexpr const char* clip_name_prefix = "Clip ";
    inline constexpr const char* section_name_prefix = "Section ";
    inline constexpr double relative_tolerance = 1e-12;
  }
  struct PlaneDefinition
  {
    std::array<double,3> origin = FilterDefaults::origin;
    std::array<double,3> normal = FilterDefaults::normal;
  };
  struct FilterDefinition
  {
    std::string id, name;
    OutputReference input;
    FilterKind kind = FilterKind::Clip;
    PlaneDefinition plane;
    ClipSide keep = ClipSide::Positive;
    std::uint64_t revision = 1;
  };
  bool normalize_plane(PlaneDefinition& plane,std::string& error);
  bool valid_filter(const FilterDefinition& filter,std::string& error);
  double signed_distance(const PlaneDefinition& plane,const std::array<double,3>& point);
}
