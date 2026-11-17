#include "ThresholdCommandText.hpp"
#include "Postprocessor/Commands/FieldCommandText.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
namespace Postprocessor
{
  QString threshold_command_text(const ResultFieldSelector& selector,const ThresholdDefinition& threshold)
  {
    QString result = field_command_text(selector);
    if (result.isEmpty())
    {
      return {};
    }
    result += " component ";
    if (threshold.field.magnitude)
    {
      result += "magnitude";
    }
    else
    {
      const auto component = plot_quote(threshold.field.component);
      if (component.isEmpty())
      {
        return {};
      }
      result += component;
    }
    if (threshold.field.dataset_id)
    {
      result += " dataset "+QString::number(static_cast<qulonglong>(*threshold.field.dataset_id));
    }
    result += " lower ";
    if (threshold.lower)
    {
      result += QString::number(*threshold.lower,'g',17);
    }
    else
    {
      result += "off";
    }
    result += " upper ";
    if (threshold.upper)
    {
      result += QString::number(*threshold.upper,'g',17);
    }
    else
    {
      result += "off";
    }
    result += " nodes ";
    if (threshold.node_rule == ThresholdNodeRule::All)
    {
      result += "all";
    }
    else
    {
      result += "any";
    }
    return result;
  }
}
