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

void TopologyManagement::createAnimationTab(QWidget* tab)
{
  auto* layout = new QVBoxLayout(tab);
  auto* selection = new QFormLayout();
  layout->addWidget(new QLabel("Export all completed iterations of the selected case.", tab));
  gifThreshold = new QDoubleSpinBox(tab);
  gifThreshold->setRange(0.0, 1.0);
  gifThreshold->setDecimals(6);
  gifThreshold->setSingleStep(0.05);
  gifThreshold->setValue(0.5);
  selection->addRow("Threshold", gifThreshold);
  layout->addLayout(selection);
  connect(gifThreshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) { setResultThreshold(value); });
  gifPath = new QLineEdit(this);
  gifPath->setPlaceholderText("Optional: default filename");
  QPushButton *browse = new QPushButton("Browse", this);
  QHBoxLayout *fileRow = new QHBoxLayout();
  fileRow->addWidget(gifPath, 1);
  fileRow->addWidget(browse);

  fps = new QSpinBox(this);
  fps->setRange(1, 100);
  fps->setValue(5);
  overwrite = new QCheckBox("Overwrite existing GIF", this);

  QFormLayout *gifForm = new QFormLayout();
  gifForm->addRow("GIF filename", fileRow);
  gifForm->addRow("Playback / GIF FPS", fps);
  gifForm->addRow(overwrite);
  layout->addLayout(gifForm);

  gifButton = new QPushButton("Export GIF", this);
  gifButton->setToolTip("Export all completed iterations of this case using the displayed threshold.");
  layout->addWidget(gifButton);
  cancelGifButton = new QPushButton("Cancel GIF export", this);
  cancelGifButton->setEnabled(false);
  layout->addWidget(cancelGifButton);
  QObject::connect(cancelGifButton, &QPushButton::clicked, this, [this]()
  {
    CalculiXDrawInterface drawing;
    drawing.cancel_topology_animation();
    cancelGifButton->setEnabled(false);
    cancelGifButton->setText("Cancelling GIF export...");
  });
  auto* animationTimer = new QTimer(this);
  QObject::connect(animationTimer, &QTimer::timeout, this, [this]()
  {
    CalculiXDrawInterface drawing;
    const bool active = drawing.topology_animation_active();
    if (active != animation_was_active)
    {
      updateDisplay();
    }
  });
  animationTimer->start(250);

  QObject::connect(deleteElementsButton, &QPushButton::clicked, this, [this]()
  {
    project(false);
  });
  QObject::connect(monitorButton, &QPushButton::clicked, this, [this]()
  {
    openJob();
  });
  QObject::connect(browse, &QPushButton::clicked, this, [this]()
  {
    browseGif();
  });
  QObject::connect(gifButton, &QPushButton::clicked, this, [this]()
  {
    exportGif();
  });
  layout->addStretch();
}

void TopologyManagement::browseGif()
{
  const QString filename = QFileDialog::getSaveFileName(this,
    "Export topology animation", gifPath->text(), "GIF animation (*.gif)",
    nullptr, QFileDialog::DontConfirmOverwrite);

  if (!filename.isEmpty())
  {
    gifPath->setText(QDir::fromNativeSeparators(filename));
  }
}

void TopologyManagement::exportGif()
{
  CalculiXDrawInterface drawing;
  pausePlayback();
  previewTimer->stop();
  if (drawing.topology_animation_active())
  {
    PRINT_ERROR("A topology GIF export is already active.\n");
    return;
  }
  const int topology_id = currentTopologyId;
  CalculiXTopologyInterface topology;
  if (topology_id <= 0 || rows.empty() || topology.get_topology_run_state(topology_id) == "running")
  {
    return;
  }
  // The animation controller validates all completed frames and their mesh data.
  const QString filename = QDir::fromNativeSeparators(gifPath->text().trimmed());

  if (filename.contains('"') || filename.contains('\n') || filename.contains('\r'))
  {
    PRINT_ERROR("The GIF filename cannot contain double quotes or line breaks.\n");
    return;
  }

  QString command = QString("ccx animate topology %1 threshold %2 fps %3")
    .arg(topology_id).arg(QString::number(threshold->value(), 'f', 6))
    .arg(fps->value());

  if (!filename.isEmpty())
  {
    command += " save \"" + filename + "\"";
  }
  if (overwrite->isChecked())
  {
    command += " overwrite";
  }

  submitCommand(command);
  updateDisplay();
}

void TopologyManagement::updateAnimationControls()
{
  CalculiXDrawInterface drawing;
  animation_was_active = drawing.topology_animation_active();
  caseTree->setEnabled(!animation_was_active);
  iterations->setEnabled(!animation_was_active);
  autoPreview->setEnabled(!animation_was_active);
  if (!animation_was_active)
  {
    cancelGifButton->setEnabled(false);
    cancelGifButton->setText("Cancel GIF export");
    return;
  }
  pausePlayback();
  for (QWidget* widget : std::initializer_list<QWidget*>{firstButton, previousButton, nextButton, lastButton,
       iterationSlider, playButton, previewButton, threshold, deleteElementsButton,
       wrapButton, gifButton, gifThreshold, wrapThreshold, wrapIterations})
  {
    widget->setEnabled(false);
  }
  if (cancelGifButton->text() != "Cancelling GIF export...")
  {
    cancelGifButton->setEnabled(true);
  }
}
