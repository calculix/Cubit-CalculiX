#include <QPen>
#include "PostprocessorAnnotationPainter.hpp"
#include <vtkRenderer.h>
#include <QPainter>
#include <QFontMetricsF>
#include <algorithm>
#include <cmath>
std::vector<QRectF> paint_result_annotations(QPainter& painter,vtkRenderer* renderer,
  const std::vector<Postprocessor::EvaluatedAnnotation>& labels,const QSize& size,double pixelRatio,double scale)
{
  std::vector<QRectF> result;
  painter.setRenderHint(QPainter::Antialiasing);
  auto font = painter.font();
  font.setPixelSize(std::max(1,static_cast<int>(std::lround(Postprocessor::AnnotationDefaults::font_size*scale))));
  painter.setFont(font);
  const QFontMetricsF metrics(font);
  const double ratio = pixelRatio;
  for (std::size_t index = 0; index < labels.size(); ++index)
  {
    const auto& value = labels[index];
    const auto text = QString::fromUtf8(value.text.c_str());
    const auto extent = metrics.boundingRect(QRectF(0,0,10000,10000),Qt::AlignLeft,text);
    const double w = std::min(extent.width()+12*scale,static_cast<double>(size.width()));
    const double h = std::min(extent.height()+8*scale,static_cast<double>(size.height()));
    const double x = std::clamp(value.position[0]*size.width(),0.0,std::max(0.0,size.width()-w));
    const double y = std::clamp((1-value.position[1])*size.height()-h,0.0,std::max(0.0,size.height()-h));
    const QRectF bounds(x,y,w,h);
    result.push_back(bounds);
    if (value.anchor)
    {
      const auto& anchor = *value.anchor;
      renderer->SetWorldPoint(anchor[0],anchor[1],anchor[2],1);
      renderer->WorldToDisplay();
      const auto* display = renderer->GetDisplayPoint();
      if (display[2] >= 0 && display[2] <= 1)
      {
        const QPointF point(display[0]/ratio,size.height()-display[1]/ratio);
        painter.setPen(QPen(QColor(70,70,70),1));
        painter.drawLine(point,bounds.center());
        painter.setBrush(QColor(190,38,209));
        painter.drawEllipse(point,3*scale,3*scale);
      }
    }
    painter.setPen(QPen(QColor(70,70,70),1));
    painter.setBrush(QColor(255,255,255,235));
    painter.drawRoundedRect(bounds,3*scale,3*scale);
    painter.setPen(Qt::black);
    painter.drawText(bounds.adjusted(6*scale,4*scale,-6*scale,-4*scale),Qt::AlignLeft | Qt::AlignTop,text);
  }
  return result;
}
