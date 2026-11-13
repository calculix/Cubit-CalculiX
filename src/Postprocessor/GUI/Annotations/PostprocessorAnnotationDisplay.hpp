#ifndef POSTPROCESSORANNOTATIONDISPLAY_HPP
#define POSTPROCESSORANNOTATIONDISPLAY_HPP
#include "Postprocessor/Core/annotations/AnnotationDefinition.hpp"
#include <QWidget>
#include <QRectF>
class vtkRenderer;
class PostprocessorApplicationInterface;
class PostprocessorAnnotationDisplay : public QWidget
{
public:
  PostprocessorAnnotationDisplay(vtkRenderer* renderer, const std::string& view, QWidget* parent);
  void synchronize(const PostprocessorApplicationInterface& application);
  std::string label_at(const QPointF& point) const;
protected:
  void paintEvent(QPaintEvent* event) override;
private:
  struct Label
  {
    Postprocessor::EvaluatedAnnotation value;
    QRectF bounds;
  };
  vtkRenderer* renderer;
  std::string viewId, activeRepresentation, activeSource;
  std::optional<std::uint64_t> geometryRevision;
  Postprocessor::AnnotationState previous;
  std::vector<Label> labels;
};
#endif
