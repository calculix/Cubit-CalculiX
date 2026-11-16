#pragma once
#include "PlotData.hpp"
namespace Postprocessor
{
  // Metadata describes already-presented sample values. Known axes normalize to
  // the first visible valid curve. Frame indices never share a physical axis.
  // Unknown units require the same scoped raw source/field or table-column basis.
  // Failure preserves data; no source array or saved plot definition is modified.
  bool bind_plot_axes(const PlotDefinition& definition,const PlotData& data,PlotAxes& axes,std::string& error);
  // Convert saved manual bounds to the current common axes. A changed quantity
  // or unknown raw-data scope falls back to fitting that axis, retaining saved bounds.
  PlotAxes presented_plot_axes(const PlotDefinition& definition,const PlotData& data);
  bool normalize_plot_axes(const PlotDefinition& definition, PlotData& data, std::string& error);
  bool record_plot_quantity(SimUnits::QuantityUnit& stored, bool& initialized,
    const SimUnits::QuantityUnit& value, std::string& error);
}
