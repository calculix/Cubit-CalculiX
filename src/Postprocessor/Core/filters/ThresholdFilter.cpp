#include "ThresholdFilter.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <vtkDataSet.h>
#include <vtkExtractCells.h>
#include <vtkUnstructuredGrid.h>
#include <vtkIdList.h>
#include <vtkNew.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <cmath>
#include <utility>
namespace Postprocessor
{
  bool ThresholdFilter::matches(double value,const ThresholdDefinition& threshold)
  {
    return std::isfinite(value) && (!threshold.lower || value >= *threshold.lower) &&
      (!threshold.upper || value <= *threshold.upper);
  }
  bool ThresholdFilter::select_cells(vtkDataSet* input,const SimResults::Increment& frame,std::size_t dataset,
    const ThresholdDefinition& threshold,std::vector<vtkIdType>& retained,std::string& warning,std::string& error)
  {
    error.clear();
    warning.clear();
    retained.clear();
    if (!input || !valid_threshold(threshold,error))
    {
      if (error.empty())
      {
        error = "The threshold input is unavailable.";
      }
      return false;
    }
    if (threshold.field.dataset_id && *threshold.field.dataset_id != dataset)
    {
      return true;
    }
    auto selection = threshold.field;
    selection.dataset_id = dataset;
    const auto* field = find_field(frame,selection);
    if (!field)
    {
      warning = "The threshold field is unavailable in this dataset/frame; output is empty.";
      return true;
    }
    SimUnits::QuantityUnit native;
    if (!SimResults::ResultsUnits::field_quantity(*field,selection.component,selection.magnitude,native,error))
    {
      return false;
    }
    const auto& bound = *threshold.bounds_unit;
    if (native.dimension != bound.dimension || native.role != bound.role ||
        native.unit.empty() != bound.unit.empty())
    {
      error = "The threshold field and saved bounds have incompatible units.";
      return false;
    }
    SimUnits::WorkspaceUnitContext presentation("postprocessor");
    if (!native.quantity.empty())
    {
      presentation.display_units[native.quantity] = native.unit;
    }
    std::vector<double> values;
    if (!color_values(input,*field,selection,values,error,&presentation))
    {
      return false;
    }
    for (auto& value : values)
    {
      if (std::isfinite(value) && !native.unit.empty())
      {
        double canonical = 0;
        if (!framework_units().to_si(value,native.unit,native.dimension,native.role,canonical,error) ||
            !framework_units().from_si(canonical,bound.unit,bound.dimension,bound.role,value,error))
        {
          return false;
        }
      }
    }
    vtkNew<vtkIdList> points;
    const bool elemental = selection.association == SimResults::Association::Element;
    for (vtkIdType cell = 0; cell < input->GetNumberOfCells(); ++cell)
    {
      bool keep = false;
      if (elemental)
      {
        keep = matches(values.at(static_cast<std::size_t>(cell)),threshold);
      }
      else
      {
        input->GetCellPoints(cell,points);
        const bool all = threshold.node_rule == ThresholdNodeRule::All ||
          selection.association == SimResults::Association::IntegrationPoint;
        keep = all && points->GetNumberOfIds() != 0;
        for (vtkIdType index = 0; index < points->GetNumberOfIds(); ++index)
        {
          const auto point = points->GetId(index);
          if (point < 0 || static_cast<std::size_t>(point) >= values.size())
          {
            error = "The threshold input contains an invalid node reference.";
            return false;
          }
          const bool match = matches(values[static_cast<std::size_t>(point)],threshold);
          if (all && !match)
          {
            keep = false;
            break;
          }
          if (!all && match)
          {
            keep = true;
            break;
          }
        }
      }
      if (keep)
      {
        retained.push_back(cell);
      }
    }
    return true;
  }
  bool ThresholdFilter::extract(vtkDataSet* input,const std::vector<vtkIdType>& cells,
    std::shared_ptr<vtkDataSet>& output,std::string& error)
  {
    vtkNew<vtkIdList> ids;
    for (const auto cell : cells)
    {
      ids->InsertNextId(cell);
    }
    vtkNew<vtkExtractCells> extract;
    extract->SetInputData(input);
    extract->SetCellList(ids);
    bool failed = false;
    vtkNew<vtkCallbackCommand> observer;
    observer->SetClientData(&failed);
    observer->SetCallback([](vtkObject*,unsigned long,void* client,void*)
    {
      *static_cast<bool*>(client) = true;
    });
    extract->AddObserver(vtkCommand::ErrorEvent,observer);
    extract->Update();
    if (failed || !extract->GetOutput() ||
        extract->GetOutput()->GetNumberOfCells() != static_cast<vtkIdType>(cells.size()))
    {
      error = "VTK could not extract the threshold cells.";
      return false;
    }
    auto result = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(),[](auto* value)
    {
      value->Delete();
    });
    result->ShallowCopy(extract->GetOutput());
    output = std::move(result);
    return true;
  }
}
