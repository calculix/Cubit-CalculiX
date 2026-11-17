#pragma once
#include "ResultColoring.hpp"
namespace Postprocessor
{
  struct VectorPlotDefinition
  {
    bool enabled = false;
    std::optional<FieldSelection> field;
    bool proportional = true;
    bool surface_only = true;
    bool magnitude_color = true;
    double scale = 1.0;
    unsigned int maximum = 2000; // Zero selects all samples.
    unsigned int stride = 1;
    std::array<double,3> color = {0.2,0.45,1.0};
  };
  bool valid_vector_plot(const VectorPlotDefinition& value,std::string& error);
  bool same_vector_plot(const VectorPlotDefinition& a,const VectorPlotDefinition& b);
}
