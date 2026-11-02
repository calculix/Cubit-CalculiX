#include "DrawingOptionsWidget.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

DrawingOptionsWidget::DrawingOptionsWidget(QWidget* parent) : QWidget(parent)
{
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  auto* group = new QGroupBox(tr("Drawing"), this);
  auto* form = new QFormLayout(group);
  sizeFactor = new QDoubleSpinBox(group);
  sizeFactor->setDecimals(4);
  sizeFactor->setRange(0.0001, 1000000.0);
  sizeFactor->setToolTip(tr("Multiplier for the automatic model-based symbol size."));
  overlayVisible = new QCheckBox(tr("Display symbols"), group);
  boundaryConditions = new QCheckBox(tr("Boundary conditions"), group);
  loads = new QCheckBox(tr("Loads"), group);
  constraints = new QCheckBox(tr("Constraints"), group);
  orientations = new QCheckBox(tr("Orientations"), group);
  hoverColor = new QComboBox(group);
  selectionColor = new QComboBox(group);
  const QStringList colors = {"red", "yellow", "green", "blue", "cyan", "magenta", "white", "black", "orange"};
  hoverColor->addItems(colors);
  selectionColor->addItems(colors);
  form->addRow(tr("Default size factor"), sizeFactor);
  form->addRow(tr("Default visibility"), overlayVisible);
  form->addRow(QString(), boundaryConditions);
  form->addRow(QString(), loads);
  form->addRow(QString(), constraints);
  form->addRow(QString(), orientations);
  form->addRow(tr("Hover color"), hoverColor);
  form->addRow(tr("Selection color"), selectionColor);
  auto* hint = new QLabel(tr("Size and visibility defaults apply to new or reset models. Colors apply to the current view."), group);
  hint->setWordWrap(true);
  form->addRow(hint);
  layout->addWidget(group);
  setOptions(DrawingOptions{});
}

void DrawingOptionsWidget::setOptions(const DrawingOptions& options)
{
  sizeFactor->setValue(options.sizeFactor);
  overlayVisible->setChecked(options.overlayVisible);
  boundaryConditions->setChecked(options.boundaryConditions);
  loads->setChecked(options.loads);
  constraints->setChecked(options.constraints);
  orientations->setChecked(options.orientations);
  hoverColor->setCurrentText(QString::fromStdString(options.hoverColor));
  selectionColor->setCurrentText(QString::fromStdString(options.selectionColor));
}

DrawingOptions DrawingOptionsWidget::options() const
{
  DrawingOptions result;
  result.sizeFactor = sizeFactor->value();
  result.overlayVisible = overlayVisible->isChecked();
  result.boundaryConditions = boundaryConditions->isChecked();
  result.loads = loads->isChecked();
  result.constraints = constraints->isChecked();
  result.orientations = orientations->isChecked();
  result.hoverColor = hoverColor->currentText().toStdString();
  result.selectionColor = selectionColor->currentText().toStdString();
  return result;
}
