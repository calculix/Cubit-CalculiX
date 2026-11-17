#include "DatasetAppearance.hpp"
#include <algorithm>
#include <cmath>
namespace Postprocessor
{
  const char* display_style_name(DisplayStyle style)
  {
    switch (style)
    {
      case DisplayStyle::Wireframe: return "wireframe";
      case DisplayStyle::HiddenLine: return "hiddenline";
      case DisplayStyle::Shaded: return "shaded";
      case DisplayStyle::Transparent: return "transparent";
      case DisplayStyle::Points: return "points";
      case DisplayStyle::RoundedPoints: return "roundedpoints";
    }
    return "";
  }
  bool parse_display_style(const std::string& text,DisplayStyle& style)
  {
    for (auto candidate : {DisplayStyle::Wireframe,DisplayStyle::HiddenLine,DisplayStyle::Shaded,
        DisplayStyle::Transparent,DisplayStyle::Points,DisplayStyle::RoundedPoints})
    {
      if (text == display_style_name(candidate))
      {
        style = candidate;
        return true;
      }
    }
    return false;
  }
  DatasetAppearance default_appearance(const SimResults::Dataset& dataset)
  {
    DatasetAppearance value;
    if (dataset.kind == "Nodesets" || dataset.kind == "DAT_Nodes" || dataset.kind == "DAT_IntegrationPoints")
    {
      value.style = DisplayStyle::Points;
      if (DisplayDefaults::rounded_points)
      {
        value.style = DisplayStyle::RoundedPoints;
      }
    }
    return value;
  }
  bool supports_style(const SimResults::Dataset& dataset,DisplayStyle style)
  {
    if (std::string(display_style_name(style)).empty())
    {
      return false;
    }
    if (style == DisplayStyle::Points || style == DisplayStyle::RoundedPoints)
    {
      return true;
    }
    return dataset.kind != "Nodesets" && dataset.kind != "DAT_Nodes" && dataset.kind != "DAT_IntegrationPoints";
  }
  bool valid_appearance(const SimResults::Dataset& dataset,const DatasetAppearance& value,std::string& error)
  {
    if (!supports_style(dataset,value.style) || !std::isfinite(value.opacity) || value.opacity <= 0 || value.opacity > 1 ||
        !std::isfinite(value.point_size) || value.point_size < 1 || value.point_size > 100 ||
        (value.solid_color && !std::all_of(value.solid_color->begin(),value.solid_color->end(),[](double channel)
        {
          return std::isfinite(channel) && channel >= 0 && channel <= 1;
        })))
    {
      error = "Incompatible display style, opacity outside (0,1], point size outside [1,100], or invalid RGB.";
      return false;
    }
    error.clear();
    return true;
  }
  void apply_appearance_patch(const AppearancePatch& patch,DatasetAppearance& value)
  {
    if (patch.style)
    {
      value.style = *patch.style;
    }
    if (patch.edges)
    {
      value.edges = *patch.edges;
    }
    if (patch.opacity)
    {
      value.opacity = *patch.opacity;
    }
    if (patch.point_size)
    {
      value.point_size = *patch.point_size;
    }
    if (patch.color_change == ColorChange::Native)
    {
      value.solid_color.reset();
    }
    else if (patch.color_change == ColorChange::Override)
    {
      value.solid_color = patch.color;
    }
  }
  bool same_appearance(const DatasetAppearance& a,const DatasetAppearance& b)
  {
    return a.style == b.style && a.edges == b.edges && a.opacity == b.opacity &&
      a.point_size == b.point_size && a.solid_color == b.solid_color;
  }
}
