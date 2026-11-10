#include "Postprocessor/Core/representations/ResultReferenceMesh.hpp"
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkFieldData.h>
#include <vtkPoints.h>
#include <vtkStringArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkExtractEdges.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkCellType.h>
#include <iostream>
namespace
{
  int failures = 0;
  void check(bool value,const char* message)
  {
    if (!value)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  }
  void kind(vtkUnstructuredGrid* mesh,const char* value)
  {
    auto array = vtkSmartPointer<vtkStringArray>::New();
    array->SetName("GroupKind"); array->InsertNextValue(value);
    mesh->GetFieldData()->AddArray(array);
  }
}
int main()
{
  constexpr long long element = 9007199254740993LL;
  auto block = vtkSmartPointer<vtkUnstructuredGrid>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  points->InsertNextPoint(0,0,0); points->InsertNextPoint(2,0,0); points->InsertNextPoint(1,0.25,0);
  block->SetPoints(points); block->Allocate(1);
  vtkIdType nodes[] = {0,1,2};
  block->InsertNextCell(VTK_QUADRATIC_EDGE,3,nodes);
  auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
  ids->SetName("OriginalElementId"); ids->InsertNextValue(element);
  block->GetCellData()->AddArray(ids); kind(block,"Blocks");
  auto ips = vtkSmartPointer<vtkUnstructuredGrid>::New();
  auto ipPoints = vtkSmartPointer<vtkPoints>::New();
  ipPoints->InsertNextPoint(0.5,0,0); ipPoints->InsertNextPoint(1.5,0,0);
  ips->SetPoints(ipPoints); ips->Allocate(2);
  for (vtkIdType i = 0; i < 2; ++i)
  {
    ips->InsertNextCell(VTK_VERTEX,1,&i);
  }
  auto parents = vtkSmartPointer<vtkTypeInt64Array>::New();
  parents->SetName("OriginalElementId"); parents->InsertNextValue(element); parents->InsertNextValue(element);
  auto ipIds = vtkSmartPointer<vtkTypeInt64Array>::New();
  ipIds->SetName("IntegrationPointId"); ipIds->InsertNextValue(1); ipIds->InsertNextValue(2);
  ips->GetPointData()->AddArray(parents); ips->GetPointData()->AddArray(ipIds);
  kind(ips,"DAT_IntegrationPoints");
  auto collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
  collection->SetPartition(0,0,block); collection->SetPartition(1,0,ips);
  collection->Register(nullptr);
  SimResults::Increment frame;
  frame.dataset = std::shared_ptr<vtkDataObject>(collection.GetPointer(),[](vtkDataObject* object)
  {
    object->Delete();
  });
  std::vector<std::shared_ptr<vtkDataSet>> output;
  std::string warning;
  check(Postprocessor::integration_point_reference_meshes(frame,ips,output,warning) && warning.empty(),"Cannot resolve IP parent element");
  check(output.size() == 1 && output.front()->GetNumberOfCells() == 1,"Repeated IP parents were not deduplicated");
  if (!output.empty())
  {
    check(output.front()->GetCellType(0) == VTK_QUADRATIC_EDGE && output.front()->GetNumberOfPoints() == 3,"Higher-order parent connectivity changed");
    double point[3]; output.front()->GetPoint(2,point);
    check(point[1] == 0.25,"Reference outline lost its original midside coordinate");
    auto edges = vtkSmartPointer<vtkExtractEdges>::New();
    edges->SetInputData(output.front().get()); edges->Update();
    check(edges->GetOutput()->GetNumberOfCells() > 0,"Parent FE edges were not extractable");
  }
  auto duplicate = vtkSmartPointer<vtkUnstructuredGrid>::New();
  duplicate->DeepCopy(block); collection->SetPartition(2,0,duplicate);
  check(Postprocessor::integration_point_reference_meshes(frame,ips,output,warning) && warning.empty() && output.size() == 1,
    "Overlapping equivalent blocks duplicated the parent element");
  duplicate->GetPoints()->SetPoint(2,1,1,0);
  check(Postprocessor::integration_point_reference_meshes(frame,ips,output,warning) && !warning.empty() && output.empty(),
    "Conflicting parent IDs selected an arbitrary block");
  parents->SetValue(0,element+1); parents->SetValue(1,element+1);
  check(Postprocessor::integration_point_reference_meshes(frame,ips,output,warning) && !warning.empty() && output.empty(),
    "Missing parent ID fabricated an outline");
  check(block->GetNumberOfCells() == 1 && ids->GetValue(0) == element,"Reference extraction mutated input topology or IDs");
  return failures ? 1 : 0;
}
