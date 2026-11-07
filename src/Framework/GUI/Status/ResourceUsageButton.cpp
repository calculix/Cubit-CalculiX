#include "ResourceUsageButton.hpp"
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionFocusRect>
#include <algorithm>
#include <cmath>

ResourceUsageButton::ResourceUsageButton(QWidget* parent) : QAbstractButton(parent)
{
  setFocusPolicy(Qt::StrongFocus);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  setCursor(Qt::PointingHandCursor);
  setAttribute(Qt::WA_Hover);
}

void ResourceUsageButton::set_usage(const QString& label, const std::vector<ResourceBarSegment>& values)
{
  setText(label);
  segments = values;
  updateGeometry();
  update();
}

QSize ResourceUsageButton::sizeHint() const
{
  return QSize((std::max)(120, fontMetrics().horizontalAdvance(text()) + 24), fontMetrics().height() + 12);
}

QSize ResourceUsageButton::minimumSizeHint() const
{
  return QSize(72, fontMetrics().height() + 12);
}

void ResourceUsageButton::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  QStyleOptionButton option;
  option.initFrom(this);
  option.rect = rect();
  option.state |= isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
  style()->drawPrimitive(QStyle::PE_PanelButtonCommand, &option, &painter, this);

  const QRect content = rect().adjusted(3, 3, -3, -3);
  double position = 0;
  painter.save();
  painter.setClipRect(content);
  for (const auto& segment : segments)
  {
    if (!std::isfinite(segment.fraction))
    {
      continue;
    }
    const double end = std::clamp(position + std::max(0.0, segment.fraction), position, 1.0);
    const int left = qRound(content.width() * position);
    const int right = qRound(content.width() * end);
    QColor color = segment.color;
    color.setAlpha(100);
    painter.fillRect(QRect(content.left() + left, content.top(), right - left, content.height()), color);
    position = end;
  }
  painter.restore();
  const QString label = fontMetrics().elidedText(text(), Qt::ElideRight, (std::max)(0, content.width() - 8));
  style()->drawItemText(&painter, content, Qt::AlignCenter, palette(), isEnabled(), label, QPalette::ButtonText);
  if (hasFocus())
  {
    QStyleOptionFocusRect focus;
    focus.initFrom(this);
    focus.rect = content;
    style()->drawPrimitive(QStyle::PE_FrameFocusRect, &focus, &painter, this);
  }
}
