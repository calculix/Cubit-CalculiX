#include "VectorPlotDefinition.hpp"
#include <cmath>
namespace Postprocessor
{
  bool valid_vector_plot(const VectorPlotDefinition& value,std::string& error)
  {
    error.clear();
    if (!std::isfinite(value.scale) || value.scale <= 0 || value.scale > 1000 ||
        value.stride == 0 || value.stride > 1000000 || value.maximum > 1000000 || (value.enabled && !value.field))
    {
      error = "Vector plots require a selected vector, scale in (0,1000], positive stride and maximum at most 1000000 (zero means all).";
      return false;
    }
    if (value.field && (!value.field->magnitude || !value.field->component.empty() ||
        value.field->association == SimResults::Association::Table || value.field->array_name.empty()))
    {
      error = "Vector display requires a spatial XYZ field selection using its physical magnitude.";
      return false;
    }
    if (value.legend.field || !valid_coloring(value.legend,error))
    {
      if (error.empty())
      {
        error = "Vector legend appearance cannot select a separate result field.";
      }
      return false;
    }
    for (double channel : value.color)
    {
      if (!std::isfinite(channel) || channel < 0 || channel > 1)
      {
        error = "Vector color channels must lie in [0,1].";
        return false;
      }
    }
    return true;
  }
  bool same_vector_plot(const VectorPlotDefinition& a,const VectorPlotDefinition& b)
  {
    return a.enabled == b.enabled && same_field(a.field,b.field) && a.proportional == b.proportional &&
      a.surface_only == b.surface_only && a.magnitude_color == b.magnitude_color && a.scale == b.scale &&
      a.maximum == b.maximum && a.stride == b.stride && a.color == b.color && same_coloring(a.legend,b.legend);
  }
}
