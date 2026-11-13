#ifndef POSTPROCESSOR_PLOTDATA_HPP
#define POSTPROCESSOR_PLOTDATA_HPP
#include "PlotDefinition.hpp"
#include <limits>
namespace Postprocessor
{
  struct PlotSample
  {
    double x = std::numeric_limits<double>::quiet_NaN();
    double y = std::numeric_limits<double>::quiet_NaN();
    double time = std::numeric_limits<double>::quiet_NaN();
    bool valid = false;
    std::optional<std::size_t> frame;
    int step = -1, increment = -1;
  };
  struct PlotCurveData
  {
    std::string name, source_name, x_title, y_title, status;
    bool time_axis = false, frame_axis = false;
    std::vector<PlotSample> samples;
  };
  struct PlotData
  {
    std::vector<PlotCurveData> curves;
  };
  bool plot_entity_data(const SimResults::VtkResultsData& data, const PlotEntityReference& reference,
    PlotCurveData& output, std::string& error);
  bool plot_table_data(std::shared_ptr<const SimResults::VtkResultsData> data, const PlotTableReference& reference,
    PlotCurveData& output, std::string& error);
  bool export_plot_csv(const PlotDefinition& definition, const PlotData& data,
    const std::string& path, std::string& error);
}
#endif
