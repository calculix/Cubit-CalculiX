#include "PostprocessorDatasetAppearance.hpp"
#include "Postprocessor/Core/representations/DisplayDefaults.hpp"
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkOpenGLState.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
#include <algorithm>
#include <cmath>
vtkStandardNewMacro(PostprocessorDepthActor);
void PostprocessorDepthActor::set_mode(int value)
{
  if (mode != value)
  {
    mode = value;
    Modified();
  }
}
void PostprocessorDepthActor::Render(vtkRenderer* renderer,vtkMapper* mapper)
{
  if (mode == 2)
  {
    return;
  }
  auto* window = vtkOpenGLRenderWindow::SafeDownCast(renderer->GetRenderWindow());
  if (mode == 1 && window)
  {
    auto* state = window->GetState();
    vtkOpenGLState::ScopedglColorMask colorMask(state);
    vtkOpenGLState::ScopedglDepthMask depthMask(state);
    state->vtkglColorMask(0,0,0,0);
    state->vtkglDepthMask(1);
    Superclass::Render(renderer,mapper);
    return;
  }
  Superclass::Render(renderer,mapper);
}
void apply_dataset_appearance(PostprocessorColorSurface& surface,const Postprocessor::DatasetAppearance& appearance,
  const SimResults::Dataset* metadata,bool visible)
{
  using Style = Postprocessor::DisplayStyle;
  auto color = Postprocessor::DisplayDefaults::surface_color;
  if (metadata && metadata->solid_color && std::all_of(metadata->solid_color->begin(),metadata->solid_color->end(),[](double channel)
    {
      return std::isfinite(channel) && channel >= 0 && channel <= 1;
    }))
  {
    color = *metadata->solid_color;
  }
  if (appearance.solid_color)
  {
    color = *appearance.solid_color;
  }
  auto* actor = PostprocessorDepthActor::SafeDownCast(surface.actor);
  int mode = 0;
  if (appearance.style == Style::HiddenLine)
  {
    mode = 1;
  }
  else if (appearance.style == Style::Wireframe || appearance.style == Style::Points || appearance.style == Style::RoundedPoints)
  {
    mode = 2;
  }
  if (actor)
  {
    actor->set_mode(mode);
  }
  surface.actor->SetVisibility(visible);
  surface.actor->GetProperty()->SetColor(color.data());
  double opacity = 1;
  if (appearance.style == Style::Transparent)
  {
    opacity = appearance.opacity;
  }
  surface.actor->GetProperty()->SetOpacity(opacity);
  surface.occludes = appearance.style == Style::HiddenLine || appearance.style == Style::Shaded;
  const bool wire = appearance.style == Style::Wireframe || appearance.style == Style::HiddenLine;
  const bool drawEdges = wire || ((appearance.style == Style::Shaded || appearance.style == Style::Transparent) && appearance.edges);
  for (const auto& edge : surface.edges)
  {
    edge->SetVisibility(visible && drawEdges);
    auto edgeColor = Postprocessor::DisplayDefaults::edge_color;
    if (wire)
    {
      edgeColor = color;
    }
    edge->GetProperty()->SetColor(edgeColor.data());
    edge->GetProperty()->SetOpacity(opacity);
    edge->GetMapper()->SetScalarVisibility(wire && surface.edge_scalars);
  }
  if (surface.points)
  {
    const bool drawPoints = appearance.style == Style::Points || appearance.style == Style::RoundedPoints;
    surface.points->SetVisibility(visible && drawPoints);
    surface.points->GetProperty()->SetColor(color.data());
    surface.points->GetProperty()->SetPointSize(appearance.point_size);
    surface.points->GetProperty()->SetRenderPointsAsSpheres(appearance.style == Style::RoundedPoints);
  }
}
