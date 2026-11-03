#include "TopologyManagement.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include <string>
#include <QString>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "TopologyDisplacementText.hpp"
#include "CubitMessage.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "GUI/jobs/JobsMonitor.hpp"
#include "CubitInterface.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QShowEvent>
#include <QHideEvent>
#include <QSlider>
#include <QTimer>
#include <QScopedValueRollback>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>

#include <algorithm>
#include <cmath>
#include <limits>
#include <initializer_list>

#include <QTreeWidget>
#include <QHeaderView>
#include <QTabWidget>
#include <QSplitter>
#include <QScreen>
#include <QGuiApplication>
#include <QCloseEvent>
#include <QScrollArea>
#include <QStandardItemModel>
#include "Core/topology/CoreTopologyStructs.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"

void TopologyManagement::createWrapTab(QWidget* tab)
{
  auto* layout = new QVBoxLayout(tab);
  auto* selection = new QFormLayout();
  wrapIterations = new QComboBox(tab);
  wrapThreshold = new QDoubleSpinBox(tab);
  wrapThreshold->setRange(0.0, 1.0);
  wrapThreshold->setDecimals(6);
  wrapThreshold->setSingleStep(0.05);
  wrapThreshold->setValue(0.5);
  selection->addRow("Result iteration", wrapIterations);
  selection->addRow("Threshold", wrapThreshold);
  layout->addLayout(selection);
  connect(wrapIterations, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
  {
    iterations->setCurrentIndex(iterations->findData(wrapIterations->currentData()));
  });
  connect(wrapThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) { setResultThreshold(value); });
  QGroupBox *wrapGroup = new QGroupBox("Alpha wrapping", this);
  QFormLayout *wrapForm = new QFormLayout(wrapGroup);

  wrapAlpha = new QLineEdit(wrapGroup);
  wrapOffset = new QLineEdit(wrapGroup);

  wrapAlpha->setPlaceholderText("Optional: calculated automatically");
  wrapOffset->setPlaceholderText("Optional: calculated automatically");

  wrapAlpha->setToolTip("Leave empty for [bounding-box diagonal / 40]. Otherwise enter a positive value in model length units.");
  wrapOffset->setToolTip("Leave empty for [resolved alpha / 30]. Otherwise enter a positive value in model length units.");

  wrapPath = new QLineEdit(wrapGroup);
  wrapPath->setPlaceholderText("Optional: default filename");

  QPushButton *wrapBrowse = new QPushButton("Browse", wrapGroup);
  QHBoxLayout *wrapFileRow = new QHBoxLayout();
  wrapFileRow->addWidget(wrapPath, 1);
  wrapFileRow->addWidget(wrapBrowse);

  wrapImport = new QCheckBox("Import STL into Cubit", wrapGroup);
  wrapImport->setChecked(true);

  wrapOverwrite = new QCheckBox("Overwrite existing STL", wrapGroup);
  wrapButton = new QPushButton("Wrap displayed iteration", wrapGroup);
  wrapButton->setToolTip("Wrap elements at or above the displayed threshold. Preserved elements are always included.");

  wrapForm->addRow("Alpha", wrapAlpha);
  wrapForm->addRow("Offset", wrapOffset);
  wrapForm->addRow("STL filename", wrapFileRow);
  wrapForm->addRow(wrapImport);
  wrapForm->addRow(wrapOverwrite);
  wrapForm->addRow(wrapButton);

  layout->addWidget(wrapGroup);

  QObject::connect(wrapBrowse, &QPushButton::clicked, this, &TopologyManagement::browseWrap);
  QObject::connect(wrapButton, &QPushButton::clicked, this, &TopologyManagement::wrapIteration);

  layout->addStretch();
}

void TopologyManagement::browseWrap()
{
  QString suggested = wrapPath->text();

  if (suggested.trimmed().isEmpty() && currentTopologyId > 0 && iterations->currentIndex() >= 0)
  {
    suggested = QString("topo_%1_iteration_%2_wrap.stl").arg(currentTopologyId).arg(iterations->currentData().toInt());
  }

  const QString filename = QFileDialog::getSaveFileName(this, "Wrapped STL", suggested, "STL files (*.stl)", nullptr, QFileDialog::DontConfirmOverwrite);

  if (!filename.isEmpty())
  {
    wrapPath->setText(QDir::fromNativeSeparators(filename));
  }
}

void TopologyManagement::wrapIteration()
{
  pausePlayback();
  previewTimer->stop();

  int topology_id;
  int iteration_id;
  int job_id;

  if (!selectedResult(topology_id, iteration_id, job_id, true))
  {
    return;
  }

  if (!meshAvailable(topology_id, iteration_id))
  {
    PRINT_ERROR("The topology iteration cannot be applied to the current mesh.\n");
    updateDisplay();
    return;
  }

  QString filename = wrapPath->text();

  if (filename.trimmed().isEmpty())
  {
    filename = QString("topo_%1_iteration_%2_wrap.stl").arg(topology_id).arg(iteration_id);
  }

  filename = QDir::fromNativeSeparators(filename);

  if (filename.contains('"') || filename.contains('\r') || filename.contains('\n'))
  {
    PRINT_ERROR("The STL filename cannot be quoted safely in a Cubit command.\n");
    return;
  }

  QString command = QString("ccx wrap topology %1 iteration %2 threshold %3")
    .arg(topology_id)
    .arg(iteration_id)
    .arg(QString::number(threshold->value(), 'g', 17));

  const auto append_parameter = [&](QLineEdit* field, const QString& keyword) -> bool
  {
    const QString text = field->text().trimmed();

    if (text.isEmpty())
    {
      return true;
    }

    bool valid = false;
    const double value = text.toDouble(&valid);

    if (!valid || !std::isfinite(value) || value <= 0.0)
    {
      PRINT_ERROR("%s must be a finite positive number, or left empty for automatic calculation.\n", keyword.toUtf8().constData());
      field->setFocus();
      field->selectAll();
      return false;
    }

    command += " " + keyword + " " + QString::number(value, 'g', 17);
    return true;
  };

  if (!append_parameter(wrapAlpha, "alpha") || !append_parameter(wrapOffset, "offset"))
  {
    return;
  }

  command += " file \"" + filename + "\"";

  if (!wrapImport->isChecked())
  {
    command += " no_import";
  }

  if (wrapOverwrite->isChecked())
  {
    command += " overwrite";
  }

  // Records the command. Its return value does not indicate command success.
  submitCommand(command);
  updateDisplay();
}

