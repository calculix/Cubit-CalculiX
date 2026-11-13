#include "PlotData.hpp"
#include <utility>
#include <algorithm>
#include <cmath>
namespace Postprocessor
{
  bool valid_plot_name(const std::string& name)
  {
    return !name.empty() && name.find_first_not_of(" ") != std::string::npos && name.size() <= 256 &&
      (name.find('"') == std::string::npos || name.find('\'') == std::string::npos) &&
      std::none_of(name.begin(),name.end(),[](unsigned char c)
      {
        return c < 32 || c == 127;
      });
  }
  bool valid_plot_axes(const PlotAxes& axes)
  {
    for (const auto& range : {axes.x,axes.y})
    {
      if (range && (!std::isfinite((*range)[0]) || !std::isfinite((*range)[1]) || (*range)[0] >= (*range)[1] || !std::isfinite((*range)[1]-(*range)[0])))
      {
        return false;
      }
    }
    return true;
  }
  bool same_plot_input(const PlotCurveDefinition& a,const PlotCurveDefinition& b)
  {
    if (a.input.index() != b.input.index())
    {
      return false;
    }
    if (const auto* left = std::get_if<PlotEntityReference>(&a.input))
    {
      const auto& right = std::get<PlotEntityReference>(b.input);
      return same_entity(left->entity,right.entity) && left->dataset_kind == right.dataset_kind &&
        left->dataset_name == right.dataset_name && left->model_id == right.model_id &&
        same_field(left->field,right.field) && left->reduction == right.reduction;
    }
    const auto& left = std::get<PlotTableReference>(a.input);
    const auto& right = std::get<PlotTableReference>(b.input);
    return left.source_id == right.source_id && left.key == right.key && left.x == right.x && left.y == right.y &&
      left.filter_column == right.filter_column && left.filter_value == right.filter_value;
  }
  const std::string& plot_curve_source(const PlotCurveDefinition& curve)
  {
    if (const auto* entity = std::get_if<PlotEntityReference>(&curve.input))
    {
      return entity->entity.source_id;
    }
    return std::get<PlotTableReference>(curve.input).source_id;
  }
}
