#include "PostprocessorAnnotationDisplay.hpp"
#include "PostprocessorAnnotationPainter.hpp"
#include <utility>
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <vtkRenderer.h>
#include <QPainter>
#include <QFontMetricsF>
#include <algorithm>

PostprocessorAnnotationDisplay::PostprocessorAnnotationDisplay(vtkRenderer* scene, const std::string& view, QWidget* parent)
  : QWidget(parent), renderer(scene), viewId(view)
{
  setAttribute(Qt::WA_TransparentForMouseEvents);
  setAttribute(Qt::WA_NoSystemBackground);
  setObjectName("Postprocessor.Annotations");
}
void PostprocessorAnnotationDisplay::synchronize(const PostprocessorApplicationInterface& application)
{
  const auto view = application.session().views.find(viewId);
  if (view == application.session().views.end())
  {
    labels.clear();
    update();
    return;
  }
  const auto& state = view->second.annotations;
  bool changed = !geometryRevision || *geometryRevision != application.display_revision() ||
    activeRepresentation != application.session().active_representation_id ||
    activeSource != application.session().active_source_id ||
    previous.labels.size() != state.labels.size() || previous.extrema_visible != state.extrema_visible || previous.visible != state.visible;
  if (!changed)
  {
    for (const auto& entry : state.labels)
    {
      const auto found = previous.labels.find(entry.first);
      if (found == previous.labels.end() || found->second.visible != entry.second.visible ||
          !Postprocessor::same_entity(found->second.entity,entry.second.entity) ||
          !Postprocessor::same_field(found->second.field,entry.second.field))
      {
        changed = true;
        break;
      }
    }
  }
  if (changed)
  {
    labels.clear();
    for (auto& value : application.annotations(viewId))
    {
      labels.push_back({std::move(value),{}});
    }
  }
  for (auto& label : labels)
  {
    if (label.value.name == "@minimum" || label.value.name == "@maximum")
    {
      label.value.position = label.value.name == "@minimum" ? state.minimum_position : state.maximum_position;
    }
    else
    {
      const auto found = state.labels.find(label.value.name);
      if (found != state.labels.end())
      {
        label.value.position = found->second.position;
      }
    }
  }
  previous = state;
  geometryRevision = application.display_revision();
  activeRepresentation = application.session().active_representation_id;
  activeSource = application.session().active_source_id;
  setGeometry(parentWidget()->rect());
  raise();
  update();
}
std::string PostprocessorAnnotationDisplay::label_at(const QPointF& point) const
{
  for (auto label = labels.rbegin(); label != labels.rend(); ++label)
  {
    if (label->bounds.contains(point))
    {
      return label->value.name;
    }
  }
  return {};
}
void PostprocessorAnnotationDisplay::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  std::vector<Postprocessor::EvaluatedAnnotation> values;
  for (const auto& label : labels)
  {
    values.push_back(label.value);
  }
  const auto bounds = paint_result_annotations(painter,renderer,values,size(),devicePixelRatioF(),1);
  for (std::size_t index = 0; index < labels.size() && index < bounds.size(); ++index)
  {
    labels[index].bounds = bounds[index];
  }
}
