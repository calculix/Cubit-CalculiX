#include "PostprocessorPicking.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Postprocessor/Core/selection/ResultProbe.hpp"
#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkCellPicker.h>
#include <vtkCamera.h>
#include <vtkDataSet.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkPointData.h>
#include <vtkPointPicker.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <algorithm>
#include <cmath>

void Postprocessor::configure_pick_provenance(vtkDataSetSurfaceFilter* filter)
{
  // Nonlinear extraction uses an intermediate surface mesh. Its pass-through
  // indices are not original input indices. Preserve the copied integral solver
  // ID arrays instead; cell attributes are copied without point interpolation.
  filter->PassThroughPointIdsOff();
  filter->PassThroughCellIdsOff();
}

bool PostprocessorScene::pick(double x, double y, Postprocessor::EntityKind kind,
  std::optional<Postprocessor::EntityReference>& entity, std::string& error)
{
  entity.reset();
  error.clear();
  auto cells = vtkSmartPointer<vtkCellPicker>::New();
  auto points = vtkSmartPointer<vtkPointPicker>::New();
  auto occluders = vtkSmartPointer<vtkCellPicker>::New();
  cells->PickFromListOn();
  points->PickFromListOn();
  points->UseCellsOn();
  occluders->PickFromListOn();
  bool hasCells = false, hasPoints = false, hasOccluders = false;
  for (const auto& display : displays)
  {
    for (const auto& surface : display.second->surfaces)
    {
      if (!surface.actor->GetVisibility())
      {
        continue;
      }
      const bool integrationPoints = surface.input->GetPointData()->HasArray("IntegrationPointId");
      if (!integrationPoints && surface.input->GetCellData()->HasArray("OriginalElementId"))
      {
        cells->AddPickList(surface.actor);
        hasCells = true;
      }
      // Only surfaces with faces occlude point picks. Vertex clouds and the
      // mesh-node marker actor must not hide one another through pick tolerance.
      if (surface.base->GetNumberOfPolys() > 0)
      {
        occluders->AddPickList(surface.actor);
        hasOccluders = true;
      }
      if (kind == Postprocessor::EntityKind::Node && !integrationPoints && surface.nodes && surface.nodes->GetVisibility())
      {
        points->AddPickList(surface.nodes);
        hasPoints = true;
      }
      else if (kind == Postprocessor::EntityKind::IntegrationPoint && integrationPoints)
      {
        points->AddPickList(surface.actor);
        hasPoints = true;
      }
    }
  }
  const auto* size = renderer->GetSize();
  if (x < 0 || y < 0 || x >= size[0] || y >= size[1])
  {
    return true;
  }
  const double tolerance = Postprocessor::SelectionDefaults::pick_radius_pixels /
    std::max(1.0,std::hypot(static_cast<double>(size[0]),static_cast<double>(size[1])));
  cells->SetTolerance(tolerance);
  points->SetTolerance(tolerance);
  vtkActor* actor = nullptr;
  vtkDataSet* picked = nullptr;
  vtkIdType index = -1;
  if (kind == Postprocessor::EntityKind::Element)
  {
    if (!hasCells || !cells->Pick(x,y,0,renderer))
    {
      return true;
    }
    actor = cells->GetActor();
    picked = cells->GetDataSet();
    index = cells->GetCellId();
  }
  else
  {
    if (!hasPoints || !points->Pick(x,y,0,renderer))
    {
      return true;
    }
    actor = points->GetActor();
    picked = points->GetDataSet();
    index = points->GetPointId();
    if (picked && index >= 0 && index < picked->GetNumberOfPoints() && hasOccluders)
    {
      double position[3];
      picked->GetPoint(index,position);
      renderer->SetWorldPoint(position[0],position[1],position[2],1);
      renderer->WorldToDisplay();
      const auto* screen = renderer->GetDisplayPoint();
      occluders->SetTolerance(1e-6);
      if (occluders->Pick(screen[0],screen[1],0,renderer))
      {
        const auto* front = occluders->GetPickPosition();
        double bounds[6];
        picked->GetBounds(bounds);
        const double diagonal = std::hypot(bounds[1]-bounds[0],bounds[3]-bounds[2],bounds[5]-bounds[4]);
        const auto* direction = renderer->GetActiveCamera()->GetDirectionOfProjection();
        const double separation = (front[0]-position[0])*direction[0] +
          (front[1]-position[1])*direction[1] + (front[2]-position[2])*direction[2];
        if (separation < -std::max(1.0,diagonal)*1e-6)
        {
          // An occluded point is a miss, not a failed command. This also allows
          // an empty click in the current pick mode to clear prior selection.
          return true;
        }
      }
    }
  }
  if (!actor || !picked || index < 0)
  {
    return true;
  }
  for (const auto& display : displays)
  {
    for (const auto& surface : display.second->surfaces)
    {
      const auto* expected = kind == Postprocessor::EntityKind::Node ? surface.nodes.GetPointer() : surface.actor.GetPointer();
      if (expected != actor)
      {
        continue;
      }
      Postprocessor::EntityReference selected;
      // Node markers retain original point tuples; surface cells retain copied
      // OriginalElementId values. No generated surface index addresses input.
      if (!Postprocessor::entity_from_index(picked,static_cast<std::size_t>(index),kind,
          display.second->source_id,surface.dataset,selected,error))
      {
        return false;
      }
      entity = selected;
      return true;
    }
  }
  error = "The picked actor is no longer associated with a result source.";
  return false;
}
