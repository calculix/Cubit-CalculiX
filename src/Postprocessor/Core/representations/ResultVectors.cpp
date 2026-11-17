#include "ResultVectors.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include <vtkDataSet.h>
#include <vtkDataSetAttributes.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <algorithm>
#include <cmath>
namespace Postprocessor
{
  bool spatial_vector(const SimResults::Field& field)
  {
    if (field.categorical || !field.vector_components || field.association == SimResults::Association::Table)
    {
      return false;
    }
    const auto& xyz = *field.vector_components;
    return xyz[0] != xyz[1] && xyz[0] != xyz[2] && xyz[1] != xyz[2] &&
      std::all_of(xyz.begin(),xyz.end(),[&](std::size_t i) { return i < field.components.size(); });
  }
  bool vector_sample(vtkDataSet* mesh,const SimResults::Field& field,std::size_t index,
    std::array<double,3>& value,std::string& unit,std::string& error,
    const SimUnits::WorkspaceUnitContext* presentation)
  {
    error.clear();
    unit.clear();
    if (!mesh || !spatial_vector(field))
    {
      error = "The field has no declared spatial XYZ vector.";
      return false;
    }
    SimUnits::QuantityUnit quantity;
    if (!SimResults::ResultsUnits::field_quantity(field,{},true,quantity,error))
    {
      return false;
    }
    vtkDataSetAttributes* attributes = mesh->GetPointData();
    vtkIdType count = mesh->GetNumberOfPoints();
    if (field.association == SimResults::Association::Element)
    {
      attributes = mesh->GetCellData();
      count = mesh->GetNumberOfCells();
    }
    auto* array = attributes->GetArray(field.array_name.c_str());
    auto* mask = attributes->GetArray(field.validity_array.c_str());
    if (!array || index >= static_cast<std::size_t>(count) || array->GetNumberOfTuples() != count ||
        array->GetNumberOfComponents() != static_cast<int>(field.components.size()) ||
        (!field.validity_array.empty() && (!mask || mask->GetNumberOfTuples() != count || mask->GetNumberOfComponents() != 1)))
    {
      error = "Vector array or validity mask is unavailable.";
      return false;
    }
    const auto tuple = static_cast<vtkIdType>(index);
    if (mask && (!std::isfinite(mask->GetComponent(tuple,0)) || mask->GetComponent(tuple,0) <= 0))
    {
      return false;
    }
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      const auto column = (*field.vector_components)[axis];
      const double native = array->GetComponent(tuple,static_cast<int>(column));
      if (!std::isfinite(native))
      {
        return false;
      }
      value[axis] = native;
      std::string displayedUnit = "unknown unit";
      if (!field.component_units.empty())
      {
        if (field.component_units.size() != field.components.size() ||
            !SimResults::ResultsUnits::display_value(native,field.component_units[column],"postprocessor",value[axis],displayedUnit,error,presentation))
        {
          if (error.empty())
          {
            error = "Vector component unit metadata is incomplete.";
          }
          return false;
        }
      }
      if (axis != 0 && displayedUnit != unit)
      {
        error = "Vector components require compatible units.";
        return false;
      }
      unit = displayedUnit;
      if (!std::isfinite(value[axis]))
      {
        return false;
      }
    }
    return true;
  }
}
