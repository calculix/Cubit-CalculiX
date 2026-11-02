#ifndef DRAWINGOPTIONSWIDGET_HPP
#define DRAWINGOPTIONSWIDGET_HPP

#include "DrawingOptions.hpp"
#include <QWidget>
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;

class DrawingOptionsWidget : public QWidget
{
public:
  explicit DrawingOptionsWidget(QWidget* parent = nullptr);
  void setOptions(const DrawingOptions& options);
  DrawingOptions options() const;
private:
  QDoubleSpinBox* sizeFactor;
  QCheckBox* overlayVisible;
  QCheckBox* boundaryConditions;
  QCheckBox* loads;
  QCheckBox* constraints;
  QCheckBox* orientations;
  QComboBox* hoverColor;
  QComboBox* selectionColor;
};

#endif
