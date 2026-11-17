#pragma once
#include "Framework/Results/VtkResultsData.hpp"
#include "Framework/Core/units/WorkspaceUnitContext.hpp"
class vtkDataSet;
namespace Postprocessor
{
  bool spatial_vector(const SimResults::Field& field);
  // Returns false for invalid samples. Errors describe incompatible metadata.
  bool vector_sample(vtkDataSet* mesh,const SimResults::Field& field,std::size_t index,
    std::array<double,3>& value,std::string& unit,std::string& error,
    const SimUnits::WorkspaceUnitContext* presentation = nullptr);
}
