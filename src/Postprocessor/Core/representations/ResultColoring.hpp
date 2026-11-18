#ifndef POSTPROCESSOR_RESULTCOLORING_HPP
#define POSTPROCESSOR_RESULTCOLORING_HPP
#include "Framework/Results/VtkResultsData.hpp"
#include <array>
#include <optional>
#include <string>
#include <vector>
#include "Framework/Core/units/WorkspaceUnitContext.hpp"
class vtkDataSet;
namespace Postprocessor
{
  enum class ColorPalette
  {
    Viridis, BlueWhiteRed, Turbo, Rainbow, Grayscale
  };
  enum class ColorRangeMode
  {
    Automatic, Fixed, Symmetric, AllFrames
  };
  namespace ColoringDefaults
  {
    inline constexpr ColorPalette palette = ColorPalette::Rainbow;
    inline constexpr std::array<double, 3> missing = {0.65, 0.65, 0.65};
    inline constexpr std::array<double, 3> minimum = {0.0, 0.0, 1.0};
    inline constexpr std::array<double, 3> maximum = {1.0, 0.0, 1.0};
    inline constexpr unsigned int maximum_bands = 256;
    inline constexpr unsigned int default_bands = 12;
    inline constexpr double legend_width_pixels = 140;
    inline constexpr double legend_height_pixels = 220;
    inline constexpr double legend_bar_pixels = 24;
    inline constexpr int legend_font_pixels = 10;
  }
  struct FieldSelection
  {
    // nullopt colors every compatible dataset in the logical result group.
    std::optional<std::size_t> dataset_id;
    std::string array_name;
    SimResults::Association association = SimResults::Association::Node;
    std::string component;
    bool magnitude = false;
    std::string group_key;
  };
  struct ColorLimit
  {
    bool enabled = false;
    double value = 0.0;
    std::array<double, 3> color = ColoringDefaults::minimum;
  };
  struct ResultColoring
  {
    std::optional<FieldSelection> field;
    ColorPalette palette = ColoringDefaults::palette;
    // Uniform control stops from palette-low to palette-high, before reversal.
    // Empty uses the built-in palette. Banded edits resample to the band count.
    std::vector<std::array<double,3>> custom_palette;
    bool palette_hsv = false; // HSV ramps require exactly two stops.
    ColorRangeMode range_mode = ColorRangeMode::AllFrames;
    std::array<double, 2> fixed_range = {0.0, 1.0};
    // Stored bounds use this presentation unit, independent of resident arrays.
    // nullopt is legacy/unbound; an empty unit explicitly denotes unknown units.
    std::optional<SimUnits::QuantityUnit> bounds_unit;
    bool reversed = false;
    bool legend_visible = true;
    std::optional<unsigned int> bands = ColoringDefaults::default_bands;
    // Bottom-left anchor in normalized viewport coordinates. nullopt uses layout.
    std::optional<std::array<double,2>> legend_position;
    ColorLimit minimum;
    ColorLimit maximum = {false, 0.0, ColoringDefaults::maximum};
  };
  struct ResultFieldGroup
  {
    SimResults::Field field;
    std::string key;
    std::vector<std::size_t> datasets;
    bool available = false;
    std::string unit_issue;
  };
  std::string field_selection_key(const FieldSelection& selection);
  // Presentation preference only: never reorder stored array components.
  // Prefer an existing MISES component (ASCII case-insensitive), otherwise
  // the first stored component. Declared vectors default to virtual magnitude.
  std::string preferred_field_component(const SimResults::Field& field);
  FieldSelection preferred_field_selection(const SimResults::Field& field, const std::string& group_key);
  std::string field_group_key(const SimResults::Field& field);
  // Group keys remain stable. Incompatible physical quantities invalidate the
  // group across frames; compatible known units use workspace presentation units.
  std::vector<ResultFieldGroup> result_field_groups(const SimResults::VtkResultsData& data,
    std::optional<std::size_t> current);
  bool same_field(const std::optional<FieldSelection>& a, const std::optional<FieldSelection>& b);
  bool same_coloring(const ResultColoring& a, const ResultColoring& b);
  bool valid_coloring(const ResultColoring& value, std::string& error);
  const SimResults::Field* find_field(const SimResults::Increment& increment, const FieldSelection& selection);
  // Values at original samples. Invalid masks and nonfinite required components
  // produce NaN. Magnitude uses only explicitly declared spatial vector columns.
  bool color_values(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::vector<double>& values, std::string& error, const SimUnits::WorkspaceUnitContext* presentation = nullptr);
  // Bounded host-thread extraction. The selected array is resolved once per
  // chunk; output owns its values and can be reduced safely on worker threads.
  bool color_values_range(vtkDataSet* mesh, const SimResults::Field& field,
    const FieldSelection& selection, std::size_t begin, std::size_t count,
    std::vector<double>& output, std::string& error);
  // A single original sample; invalid masks/nonfinite values produce NaN.
  bool color_sample(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::size_t index, double& value, std::string& error, const SimUnits::WorkspaceUnitContext* presentation = nullptr);
  // Accumulate only finite valid original samples, without allocating a scalar copy.
  bool accumulate_color_range(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::optional<std::array<double,2>>& range, std::string& error, const SimUnits::WorkspaceUnitContext* presentation = nullptr);
  // Stable preference names; independent of translated GUI labels.
  const char* color_palette_name(ColorPalette palette);
  bool parse_color_palette(const std::string& name, ColorPalette& palette);
  // Sampling assumes valid_coloring; stops are uniform and before reversal.
  // set_palette_stop validates inputs and uses one-based indices.
  std::array<double,3> coloring_palette_color(const ResultColoring& coloring,double t);
  std::vector<std::array<double,3>> palette_endpoints(const ResultColoring& coloring);
  bool set_palette_endpoint(ResultColoring& coloring,bool high,const std::array<double,3>& rgb,std::string& error);
  void reset_custom_palette(ResultColoring& coloring);
  std::vector<std::array<double,3>> palette_samples(const ResultColoring& coloring);
  bool set_palette_stop(ResultColoring& coloring,std::size_t stop,const std::array<double,3>& rgb,std::string& error);
  std::array<double, 3> palette_color(ColorPalette palette, double position);
  std::array<double, 3> result_color(double value, const ResultColoring& coloring, const std::array<double, 2>& range);
}
#endif
