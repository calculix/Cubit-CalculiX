#include "FilterRuntime.hpp"
#include <vtkCutter.h>
#include <vtkPlane.h>
#include <vtkPolyData.h>
#include <vtkNew.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
namespace Postprocessor
{
  bool evaluate_section(vtkDataSet* input,const FilterDefinition& filter,std::shared_ptr<vtkDataSet>& output,std::string& error)
  {
    vtkNew<vtkPlane> plane;
    plane->SetOrigin(filter.plane.origin.data());
    plane->SetNormal(filter.plane.normal.data());
    vtkNew<vtkCutter> cut;
    cut->SetInputData(input);
    cut->SetCutFunction(plane);
    cut->SetValue(0,0);
    cut->GenerateTrianglesOff();
    bool failed = false;
    vtkNew<vtkCallbackCommand> observer;
    observer->SetClientData(&failed);
    observer->SetCallback([](vtkObject*,unsigned long,void* client,void*)
    {
      *static_cast<bool*>(client) = true;
    });
    cut->AddObserver(vtkCommand::ErrorEvent,observer);
    cut->Update();
    if (failed || !cut->GetOutput())
    {
      error = "VTK could not evaluate the section plane.";
      return false;
    }
    auto result = std::shared_ptr<vtkPolyData>(vtkPolyData::New(),[](auto* value)
    {
      value->Delete();
    });
    result->ShallowCopy(cut->GetOutput());
    output = result;
    return true;
  }
}
