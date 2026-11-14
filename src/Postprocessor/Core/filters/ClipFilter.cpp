#include "FilterRuntime.hpp"
#include <vtkTableBasedClipDataSet.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPlane.h>
#include <vtkNew.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
namespace Postprocessor
{
  bool evaluate_clip(vtkDataSet* input,const FilterDefinition& filter,std::shared_ptr<vtkDataSet>& output,std::string& error)
  {
    vtkNew<vtkPlane> plane;
    plane->SetOrigin(filter.plane.origin.data());
    plane->SetNormal(filter.plane.normal.data());
    vtkNew<vtkTableBasedClipDataSet> clip;
    clip->SetInputData(input);
    clip->SetClipFunction(plane);
    clip->SetValue(0);
    clip->SetInsideOut(filter.keep == ClipSide::Negative);
    clip->GenerateClipScalarsOff();
    bool failed = false;
    vtkNew<vtkCallbackCommand> observer;
    observer->SetClientData(&failed);
    observer->SetCallback([](vtkObject*,unsigned long,void* client,void*)
    {
      *static_cast<bool*>(client) = true;
    });
    clip->AddObserver(vtkCommand::ErrorEvent,observer);
    clip->Update();
    if (failed || !clip->GetOutput())
    {
      error = "VTK could not evaluate the clipping plane.";
      return false;
    }
    auto result = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(),[](auto* value)
    {
      value->Delete();
    });
    result->ShallowCopy(clip->GetOutput());
    output = result;
    return true;
  }
}
