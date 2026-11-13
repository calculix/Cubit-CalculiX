#ifndef POSTPROCESSOR_PLOTDEFINITION_HPP
#define POSTPROCESSOR_PLOTDEFINITION_HPP
#include "../selection/ResultProbe.hpp"
#include <variant>
#include <map>
namespace Postprocessor
{
  namespace PlotDefaults
  {
    inline constexpr std::size_t maximum_plots = 64;
    inline constexpr std::size_t maximum_curves = 64;
    inline constexpr std::size_t maximum_samples = 1000000;
    inline constexpr bool legend_visible = true;
    inline constexpr bool grid_visible = true;
    inline constexpr std::array<double,3> background = {1.0,1.0,1.0};
    inline constexpr float line_width = 1.5f;
    inline constexpr float marker_size = 4.0f;
    inline constexpr std::array<std::array<unsigned char,3>,6> colors = {{{55,110,210},{210,65,50},{35,150,95},{160,65,190},{210,145,25},{30,160,180}}};
    inline constexpr std::array<int,3> cursor_color = {120,120,120};
    inline constexpr int wheel_commit_ms = 150;
  }
  enum class PlotReduction
  {
    None, Minimum, Maximum
  };
  struct PlotEntityReference
  {
    EntityReference entity;
    std::string dataset_kind, dataset_name;
    int model_id = -1;
    FieldSelection field;
    PlotReduction reduction = PlotReduction::None;
  };
  struct PlotTableReference
  {
    std::string source_id, key, x, y, filter_column, filter_value;
  };
  struct PlotCurveDefinition
  {
    std::string name;
    bool visible = true;
    std::array<unsigned char,3> color = PlotDefaults::colors[0];
    std::variant<PlotEntityReference,PlotTableReference> input;
  };
  struct PlotAxes
  {
    // Linear axes only. Missing bounds request automatic fitting.
    std::optional<std::array<double,2>> x, y;
  };
  struct PlotDefinition
  {
    std::string id, name;
    std::vector<PlotCurveDefinition> curves;
    PlotAxes axes;
    // Runtime cache token, never serialized.
    std::uint64_t data_revision = 1;
  };
  bool valid_plot_name(const std::string& name);
  bool valid_plot_axes(const PlotAxes& axes);
  bool same_plot_input(const PlotCurveDefinition& a,const PlotCurveDefinition& b);
  const std::string& plot_curve_source(const PlotCurveDefinition& curve);
  struct PostprocessorSession;
  bool prune_plot_sources(PostprocessorSession& state,const std::string& source,std::string& error);
}
#endif
