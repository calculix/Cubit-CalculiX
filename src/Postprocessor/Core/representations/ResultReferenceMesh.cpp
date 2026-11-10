#include "ResultReferenceMesh.hpp"
#include "ResultMesh.hpp"
#include <vtkAlgorithm.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkExtractCells.h>
#include <vtkFieldData.h>
#include <vtkIdList.h>
#include <vtkPointData.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVariant.h>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <array>
#include <cstdint>
#include <exception>
#include <map>
#include <set>
namespace
{
  bool id(vtkDataArray* values, vtkIdType index, std::int64_t& result)
  {
    if (!values || values->GetNumberOfComponents() != 1 || index < 0 || index >= values->GetNumberOfTuples() ||
        values->GetDataType() == VTK_FLOAT || values->GetDataType() == VTK_DOUBLE)
    {
      return false;
    }
    bool valid = false;
    result = values->GetVariantValue(index).ToLongLong(&valid);
    return valid && result > 0;
  }
  bool parent_mesh(vtkDataSet* mesh)
  {
    if (mesh->GetPointData()->HasArray("IntegrationPointId"))
    {
      return false;
    }
    auto* kind = vtkStringArray::SafeDownCast(mesh->GetFieldData()->GetAbstractArray("GroupKind"));
    if (kind && kind->GetNumberOfValues() != 0)
    {
      const std::string value = kind->GetValue(0);
      return value == "Blocks" || value == "DAT_Elements";
    }
    // Generic imported grids may supply original IDs without group metadata.
    return true;
  }
  struct Cell
  {
    vtkDataSet* mesh = nullptr;
    vtkIdType index = 0;
    bool ambiguous = false;
  };
  bool same_cell(const Cell& a, vtkDataSet* mesh, vtkIdType index)
  {
    if (a.mesh->GetCellType(a.index) != mesh->GetCellType(index))
    {
      return false;
    }
    auto left = vtkSmartPointer<vtkIdList>::New();
    auto right = vtkSmartPointer<vtkIdList>::New();
    a.mesh->GetCellPoints(a.index,left);
    mesh->GetCellPoints(index,right);
    if (left->GetNumberOfIds() != right->GetNumberOfIds())
    {
      return false;
    }
    auto* leftIds = a.mesh->GetPointData()->GetArray("OriginalNodeId");
    auto* rightIds = mesh->GetPointData()->GetArray("OriginalNodeId");
    if ((leftIds != nullptr) != (rightIds != nullptr))
    {
      return false;
    }
    for (vtkIdType node = 0; node < left->GetNumberOfIds(); ++node)
    {
      if (leftIds)
      {
        std::int64_t leftId = 0, rightId = 0;
        if (!id(leftIds,left->GetId(node),leftId) || !id(rightIds,right->GetId(node),rightId) || leftId != rightId)
        {
          return false;
        }
      }
      std::array<double,3> first{}, second{};
      a.mesh->GetPoint(left->GetId(node),first.data());
      mesh->GetPoint(right->GetId(node),second.data());
      if (first != second)
      {
        return false;
      }
    }
    return true;
  }
}
bool Postprocessor::integration_point_reference_meshes(const SimResults::Increment& frame,
  vtkDataSet* integrationPoints, std::vector<std::shared_ptr<vtkDataSet>>& output, std::string& warning)
{
  warning.clear();
  try
  {
    std::set<std::int64_t> wanted;
    auto* parents = integrationPoints ? integrationPoints->GetPointData()->GetArray("OriginalElementId") : nullptr;
    bool invalid = false;
    if (!integrationPoints || !integrationPoints->GetPointData()->HasArray("IntegrationPointId"))
    {
      warning = "Reference outline requires an integration-point dataset.";
      output.clear();
      return true;
    }
    for (vtkIdType point = 0; point < integrationPoints->GetNumberOfPoints(); ++point)
    {
      std::int64_t value = 0;
      if (id(parents,point,value))
      {
        wanted.insert(value);
      }
      else
      {
        invalid = true;
      }
    }
    std::map<std::int64_t,Cell> matched;
    for (const auto& part : result_mesh_parts(frame.dataset.get()))
    {
      auto* mesh = part.second;
      if (!parent_mesh(mesh))
      {
        continue;
      }
      auto* elements = mesh->GetCellData()->GetArray("OriginalElementId");
      for (vtkIdType cell = 0; cell < mesh->GetNumberOfCells(); ++cell)
      {
        std::int64_t value = 0;
        if (!id(elements,cell,value) || wanted.count(value) == 0 || mesh->GetCell(cell)->GetCellDimension() == 0)
        {
          continue;
        }
        const auto found = matched.find(value);
        if (found == matched.end())
        {
          matched.emplace(value,Cell{mesh,cell,false});
        }
        else if (!found->second.ambiguous && !same_cell(found->second,mesh,cell))
        {
          found->second.ambiguous = true;
        }
      }
    }
    std::map<vtkDataSet*,std::vector<vtkIdType>> selections;
    std::size_t resolved = 0;
    for (const auto& item : matched)
    {
      if (!item.second.ambiguous)
      {
        selections[item.second.mesh].push_back(item.second.index);
        ++resolved;
      }
    }
    std::vector<std::shared_ptr<vtkDataSet>> extracted;
    for (const auto& selection : selections)
    {
      auto cells = vtkSmartPointer<vtkIdList>::New();
      for (auto index : selection.second)
      {
        cells->InsertNextId(index);
      }
      auto filter = vtkSmartPointer<vtkExtractCells>::New();
      filter->SetInputData(selection.first);
      filter->SetCellList(cells);
      filter->PassThroughCellIdsOff();
      filter->SetOutputPointsPrecision(vtkAlgorithm::DOUBLE_PRECISION);
      bool failed = false;
      auto observer = vtkSmartPointer<vtkCallbackCommand>::New();
      observer->SetClientData(&failed);
      observer->SetCallback([](vtkObject*,unsigned long,void* client,void*)
      {
        *static_cast<bool*>(client) = true;
      });
      filter->AddObserver(vtkCommand::ErrorEvent,observer);
      filter->Update();
      if (failed || !filter->GetOutput() || filter->GetOutput()->GetNumberOfCells() != cells->GetNumberOfIds())
      {
        warning = "Cannot extract undeformed integration-point parent cells.";
        return false;
      }
      auto mesh = std::shared_ptr<vtkDataSet>(vtkUnstructuredGrid::New(),[](vtkDataSet* value)
      {
        value->Delete();
      });
      mesh->ShallowCopy(filter->GetOutput());
      extracted.push_back(std::move(mesh));
    }
    if (invalid || resolved != wanted.size())
    {
      warning = "Some integration-point parents are missing or ambiguous; their undeformed outlines are unavailable.";
    }
    output.swap(extracted);
    return true;
  }
  catch (const std::exception& exception)
  {
    warning = std::string("Cannot prepare undeformed integration-point parents: ")+exception.what();
    return false;
  }
}
