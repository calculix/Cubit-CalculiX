#ifndef POSTPROCESSOR_RESULTCOLORING_HPP
#define POSTPROCESSOR_RESULTCOLORING_HPP
#include "Framework/Results/VtkResultsData.hpp"
#include <array>
#include <optional>
#include <string>
#include <vector>
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
    ColorRangeMode range_mode = ColorRangeMode::Automatic;
    std::array<double, 2> fixed_range = {0.0, 1.0};
    bool reversed = false;
    bool legend_visible = true;
    std::optional<unsigned int> bands;
    ColorLimit minimum;
    ColorLimit maximum = {false, 0.0, ColoringDefaults::maximum};
  };
  struct ResultFieldGroup
  {
    SimResults::Field field;
    std::string key;
    std::vector<std::size_t> datasets;
    bool available = false;
  };
  std::string field_selection_key(const FieldSelection& selection);
  std::string field_group_key(const SimResults::Field& field);
  std::vector<ResultFieldGroup> result_field_groups(const SimResults::VtkResultsData& data,
    std::optional<std::size_t> current);
  bool same_field(const std::optional<FieldSelection>& a, const std::optional<FieldSelection>& b);
  bool same_coloring(const ResultColoring& a, const ResultColoring& b);
  bool valid_coloring(const ResultColoring& value, std::string& error);
  const SimResults::Field* find_field(const SimResults::Increment& increment, const FieldSelection& selection);
  // Values at original samples. Invalid masks and nonfinite required components
  // produce NaN. Magnitude uses only explicitly declared spatial vector columns.
  bool color_values(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::vector<double>& values, std::string& error);
  // A single original sample; invalid masks/nonfinite values produce NaN.
  bool color_sample(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::size_t index, double& value, std::string& error);
  // Accumulate only finite valid original samples, without allocating a scalar copy.
  bool accumulate_color_range(vtkDataSet* mesh, const SimResults::Field& field, const FieldSelection& selection,
    std::optional<std::array<double,2>>& range, std::string& error);
  // Stable preference names; independent of translated GUI labels.
  const char* color_palette_name(ColorPalette palette);
  bool parse_color_palette(const std::string& name, ColorPalette& palette);
  std::array<double, 3> palette_color(ColorPalette palette, double position);
  std::array<double, 3> result_color(double value, const ResultColoring& coloring, const std::array<double, 2>& range);
}
#endif
