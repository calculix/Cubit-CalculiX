#pragma once
#include "../representations/ResultColoring.hpp"
#include <optional>
namespace SimUnits
{
  class CoreFrameworkUnits;
}
namespace Postprocessor
{
  enum class ThresholdNodeRule { All, Any };
  struct ThresholdDefinition
  {
    FieldSelection field;
    std::optional<double> lower, upper;
    std::optional<SimUnits::QuantityUnit> bounds_unit;
    ThresholdNodeRule node_rule = ThresholdNodeRule::All;
    // Stable dataset identity; numeric IDs may change after refresh/restore.
    std::string dataset_kind, dataset_name;
    int dataset_model = -1;
  };
  bool valid_threshold(const ThresholdDefinition& value,std::string& error);
  bool bind_threshold_units(ThresholdDefinition& value,const SimResults::Field& field,std::string& error);
  bool convert_threshold_units(ThresholdDefinition& value,const SimUnits::CoreFrameworkUnits& units,std::string& error);
}
