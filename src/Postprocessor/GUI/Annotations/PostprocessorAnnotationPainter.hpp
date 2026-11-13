#ifndef POSTPROCESSORANNOTATIONPAINTER_HPP
#define POSTPROCESSORANNOTATIONPAINTER_HPP
#include "Postprocessor/Core/annotations/AnnotationDefinition.hpp"
#include <QRectF>
#include <QSize>
class QPainter;
class vtkRenderer;
// Paint without touching GUI widgets. Ratio converts VTK pixels to painter units.
std::vector<QRectF> paint_result_annotations(QPainter& painter,vtkRenderer* renderer,
  const std::vector<Postprocessor::EvaluatedAnnotation>& labels,const QSize& size,double pixelRatio,double scale);
#endif
