#include "DrawingOptionsWidget.hpp"
#include "UserOptions.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
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
  labelsEnabled = new QCheckBox(tr("Display labels"), group);
  form->addRow(QString(), labelsEnabled);
  labelsLoads = new QCheckBox(tr("Load labels"), group);
  form->addRow(QString(), labelsLoads);
  labelsBcs = new QCheckBox(tr("BC labels"), group);
  form->addRow(QString(), labelsBcs);
  labelsSelectedOnly = new QCheckBox(tr("Selected entities only"), group);
  form->addRow(QString(), labelsSelectedOnly);
  labelsIds = new QCheckBox(tr("Show IDs"), group);
  form->addRow(QString(), labelsIds);
  labelsNames = new QCheckBox(tr("Show names"), group);
  form->addRow(QString(), labelsNames);
  labelsValues = new QCheckBox(tr("Show defined values"), group);
  form->addRow(QString(), labelsValues);
  labelsAmplitudes = new QCheckBox(tr("Show amplitude references"), group);
  form->addRow(QString(), labelsAmplitudes);
  labelsLeaders = new QCheckBox(tr("Leader lines"), group);
  form->addRow(QString(), labelsLeaders);
  labelsOffset = new QDoubleSpinBox(group);
  labelsOffset->setRange(0, 100);
  labelsOffset->setDecimals(3);
  labelsColor = new QComboBox(group);
  labelsColor->addItems(colors);
  form->addRow(tr("Label offset factor"), labelsOffset);
  form->addRow(tr("Label color"), labelsColor);
  auto* hint = new QLabel(tr("Size and visibility defaults apply to new or reset models. Colors apply to the current view."), group);
  hint->setWordWrap(true);
  form->addRow(hint);
  layout->addWidget(group);
  setOptions(CalculiXApplicationInterface().user_options().mDrawing);
}

void DrawingOptionsWidget::setOptions(const DrawingOptions& options)
{
  labelsEnabled->setChecked(options.labelsEnabled);
  labelsLoads->setChecked(options.labelsLoads);
  labelsBcs->setChecked(options.labelsBcs);
  labelsSelectedOnly->setChecked(options.labelsSelectedOnly);
  labelsIds->setChecked(options.labelsIds);
  labelsNames->setChecked(options.labelsNames);
  labelsValues->setChecked(options.labelsValues);
  labelsAmplitudes->setChecked(options.labelsAmplitudes);
  labelsLeaders->setChecked(options.labelsLeaders);
  labelsOffset->setValue(options.labelsOffset);
  labelsColor->setCurrentText(QString::fromStdString(options.labelsColor));
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
  result.labelsEnabled = labelsEnabled->isChecked();
  result.labelsLoads = labelsLoads->isChecked();
  result.labelsBcs = labelsBcs->isChecked();
  result.labelsSelectedOnly = labelsSelectedOnly->isChecked();
  result.labelsIds = labelsIds->isChecked();
  result.labelsNames = labelsNames->isChecked();
  result.labelsValues = labelsValues->isChecked();
  result.labelsAmplitudes = labelsAmplitudes->isChecked();
  result.labelsLeaders = labelsLeaders->isChecked();
  result.labelsOffset = labelsOffset->value();
  result.labelsColor = labelsColor->currentText().toStdString();
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
