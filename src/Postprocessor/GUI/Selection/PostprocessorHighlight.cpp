#include <type_traits>
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <vtkActor.h>
#include <vtkCell.h>
#include <vtkCellArray.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkExtractEdges.h>
#include <vtkIdList.h>
#include <vtkOpenGLActor.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkOpenGLProperty.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>

void PostprocessorScene::update_selection(const PostprocessorApplicationInterface& application)
{
  const auto view = application.session().views.find(viewId);
  if (view == application.session().views.end())
  {
    return;
  }
  const auto& selection = view->second.selection;
  if (highlightedSelection == selection.revision && highlightedGeometry == application.display_revision())
  {
    return;
  }
  clear_hover();
  if (highlight)
  {
    highlight->VisibilityOff();
  }
  highlightedSelection = selection.revision;
  highlightedGeometry = application.display_revision();
  if (!selection.entity)
  {
    if (highlight)
    {
      renderer->RemoveActor(highlight);
      highlight = nullptr;
    }
    return;
  }
  build_highlight(*selection.entity,application,highlight,Postprocessor::SelectionDefaults::highlight_color);
}

bool PostprocessorScene::clear_hover()
{
  const bool changed = hoveredEntity.has_value();
  hoveredEntity.reset();
  if (hoverHighlight)
  {
    renderer->RemoveActor(hoverHighlight);
    hoverHighlight = nullptr;
  }
  return changed;
}

bool PostprocessorScene::set_hover(const std::optional<Postprocessor::EntityReference>& entity,
  const PostprocessorApplicationInterface& application)
{
  if (!entity)
  {
    return clear_hover();
  }
  if (hoveredEntity && Postprocessor::same_entity(*hoveredEntity,*entity))
  {
    return false;
  }
  clear_hover();
  hoveredEntity = entity;
  build_highlight(*entity,application,hoverHighlight,Postprocessor::SelectionDefaults::hover_color);
  return true;
}

void PostprocessorScene::build_highlight(const Postprocessor::EntityReference& entity,
  const PostprocessorApplicationInterface& application, vtkSmartPointer<vtkActor>& actor,
  const std::array<double,3>& color)
{
  const auto active = application.session().representations.find(application.session().active_representation_id);
  if (entity.kind == Postprocessor::EntityKind::Element && active != application.session().representations.end() &&
      active->second.view_id == viewId && active->second.input.owner == Postprocessor::OutputOwner::Feature &&
      application.output_source(active->second.input) == entity.source_id)
  {
    std::string error;
    const auto lines = application.filtered_entity_edges(active->second,entity,error);
    if (!lines || lines->GetNumberOfCells() == 0)
    {
      return;
    }
    auto mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
    mapper->SetInputData(lines.get());
    mapper->ScalarVisibilityOff();
    if (!actor)
    {
      actor = vtkSmartPointer<vtkOpenGLActor>::New();
      actor->PickableOff();
      actor->UseBoundsOff();
      renderer->AddActor(actor);
    }
    double factor = 1;
    bool known = false;
    if (!application.scene_scale(active->second,factor,known,error))
    {
      return;
    }
    actor->SetScale(factor,factor,factor);
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color[0],color[1],color[2]);
    actor->GetProperty()->SetLineWidth(Postprocessor::SelectionDefaults::highlight_line_width);
    actor->GetProperty()->LightingOff();
    actor->VisibilityOn();
    return;
  }
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->SetDataTypeToDouble();
  auto vertices = vtkSmartPointer<vtkCellArray>::New();
  auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  grid->SetPoints(points);
  grid->Allocate();
  for (const auto& entry : displays)
  {
    const auto definition = application.session().representations.find(entry.first);
    if (definition == application.session().representations.end() || entry.second->source_id != entity.source_id ||
        !Postprocessor::dataset_visible(definition->second,entity.dataset_id))
    {
      continue;
    }
    for (const auto& surface : entry.second->surfaces)
    {
      if (surface.dataset != entity.dataset_id)
      {
        continue;
      }
      std::string error;
      auto* input = [&]() -> std::common_type_t<decltype(static_cast<vtkDataSet*>(surface.node_data.GetPointer())), decltype(surface.input)> {
        if (entity.kind == Postprocessor::EntityKind::Node && surface.node_data) {
          return static_cast<vtkDataSet*>(surface.node_data.GetPointer());
        } else {
          return surface.input;
        }
      }();
      for (auto index : Postprocessor::entity_indices(input,entity,error))
      {
        if (entity.kind != Postprocessor::EntityKind::Element)
        {
          double position[3];
          input->GetPoint(static_cast<vtkIdType>(index),position);
          for (auto& coordinate : position)
          {
            coordinate *= entry.second->scene_scale;
          }
          const auto id = points->InsertNextPoint(position);
          vertices->InsertNextCell(1,&id);
        }
        else
        {
          auto* cell = surface.input->GetCell(static_cast<vtkIdType>(index));
          if (!cell || cell->GetCellType() == VTK_POLYHEDRON)
          {
            continue;
          }
          auto local = vtkSmartPointer<vtkIdList>::New();
          for (vtkIdType i = 0; i < cell->GetNumberOfPoints(); ++i)
          {
            double position[3];
            surface.input->GetPoint(cell->GetPointId(i),position);
            for (auto& coordinate : position)
            {
              coordinate *= entry.second->scene_scale;
            }
            local->InsertNextId(points->InsertNextPoint(position));
          }
          grid->InsertNextCell(cell->GetCellType(),local);
        }
      }
    }
  }
  if (points->GetNumberOfPoints() == 0)
  {
    return;
  }
  auto mapper = vtkSmartPointer<vtkOpenGLPolyDataMapper>::New();
  mapper->ScalarVisibilityOff();
  if (entity.kind == Postprocessor::EntityKind::Element)
  {
    auto edges = vtkSmartPointer<vtkExtractEdges>::New();
    edges->SetInputData(grid);
    edges->Update();
    mapper->SetInputConnection(edges->GetOutputPort());
    mapper->SetRelativeCoincidentTopologyLineOffsetParameters(0,-2);
  }
  else
  {
    auto data = vtkSmartPointer<vtkPolyData>::New();
    data->SetPoints(points);
    data->SetVerts(vertices);
    mapper->SetInputData(data);
    mapper->SetRelativeCoincidentTopologyPointOffsetParameter(-2);
  }
  if (!actor)
  {
    actor = vtkSmartPointer<vtkOpenGLActor>::New();
    actor->PickableOff();
    actor->UseBoundsOff();
    renderer->AddActor(actor);
  }
  auto property = vtkSmartPointer<vtkOpenGLProperty>::New();
  property->SetColor(color[0],color[1],color[2]);
  property->SetLineWidth(Postprocessor::SelectionDefaults::highlight_line_width);
  property->SetPointSize(Postprocessor::SelectionDefaults::highlight_point_size);
  property->SetRenderPointsAsSpheres(true);
  property->LightingOff();
  actor->SetScale(1,1,1);
  actor->SetProperty(property);
  actor->SetMapper(mapper);
  actor->VisibilityOn();
}
