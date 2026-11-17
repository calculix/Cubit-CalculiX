#pragma once
#include "ThresholdDefinition.hpp"
#include <vtkType.h>
#include <memory>
#include <vector>
class vtkDataSet;
namespace Postprocessor
{
  class ThresholdFilter
  {
  public:
    static bool matches(double value,const ThresholdDefinition& threshold);
    static bool select_cells(vtkDataSet* input,const SimResults::Increment& frame,std::size_t dataset,
      const ThresholdDefinition& threshold,std::vector<vtkIdType>& retained,std::string& warning,std::string& error);
    static bool extract(vtkDataSet* input,const std::vector<vtkIdType>& cells,
      std::shared_ptr<vtkDataSet>& output,std::string& error);
  };
}
