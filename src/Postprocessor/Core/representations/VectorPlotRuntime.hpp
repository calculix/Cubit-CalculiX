#pragma once
#include "Representation.hpp"
#include <cstdint>
namespace Postprocessor
{
  struct VectorPlotSample
  {
    std::array<double,3> position, direction;
    double magnitude = 0, length = 0;
    std::size_t dataset = 0;
    long long entity = -1, integration_point = -1;
    std::uint64_t key = 0;
  };
  struct VectorPlotData
  {
    std::vector<VectorPlotSample> samples;
    std::string unit, warning;
    double reference_length = 0, reference_magnitude = 0;
    std::array<double,6> bounds = {1,-1,1,-1,1,-1};
  };
  // Source supplies stable scale references; displayed contains filtered geometry.
  bool prepare_vector_plot(const SimResults::VtkResultsData& source,const SimResults::VtkResultsData& displayed,
    const Representation& display,double referenceMagnitude,VectorPlotData& output,std::string& error,
    const SimUnits::WorkspaceUnitContext* presentation = nullptr);
}
