#pragma once
#include "DisplayDefaults.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
namespace Postprocessor
{
  enum class DisplayStyle
  {
    Wireframe, HiddenLine, Shaded, Transparent, Points, RoundedPoints
  };
  namespace AppearanceDefaults
  {
    inline constexpr double opacity = 0.35;
    inline constexpr bool edges = true;
  }
  struct DatasetAppearance
  {
    DisplayStyle style = DisplayStyle::Shaded;
    bool edges = AppearanceDefaults::edges;
    double opacity = AppearanceDefaults::opacity;
    double point_size = DisplayDefaults::point_size_pixels;
    std::optional<std::array<double,3>> solid_color;
  };
  enum class ColorChange
  {
    Unchanged, Override, Native
  };
  struct AppearancePatch
  {
    std::optional<DisplayStyle> style;
    std::optional<bool> edges;
    std::optional<double> opacity,point_size;
    ColorChange color_change = ColorChange::Unchanged;
    std::array<double,3> color{};
    bool reset = false;
  };
  struct AppearanceTarget
  {
    std::string source_id,view_id;
    std::size_t dataset_id = 0;
  };
  DatasetAppearance default_appearance(const SimResults::Dataset& dataset);
  bool supports_style(const SimResults::Dataset& dataset,DisplayStyle style);
  bool valid_appearance(const SimResults::Dataset& dataset,const DatasetAppearance& value,std::string& error);
  void apply_appearance_patch(const AppearancePatch& patch,DatasetAppearance& value);
  bool same_appearance(const DatasetAppearance& a,const DatasetAppearance& b);
  const char* display_style_name(DisplayStyle style);
  bool parse_display_style(const std::string& text,DisplayStyle& style);
}
