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

void TopologyManagement::createIterationsTab(QWidget* tab)
{
  auto* layout = new QVBoxLayout(tab);
  iterations = new QComboBox(tab);
  auto* selection = new QFormLayout();
  selection->addRow("Result iteration", iterations);
  layout->addLayout(selection);
  threshold = new QDoubleSpinBox(tab);
  threshold->setRange(0.0, 1.0);
  threshold->setDecimals(6);
  threshold->setSingleStep(0.05);
  threshold->setValue(0.5);
  iterationTree = new QTreeWidget(tab);
  iterationTree->setRootIsDecorated(false);
  iterationTree->setHeaderLabels({"Iteration", "Physical job", "Status", "Objective", "Volume", "Converged"});
  iterationTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  layout->addWidget(iterationTree, 1);
  connect(iterationTree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* item, QTreeWidgetItem*)
  {
    if (item != nullptr)
    {
      iterations->setCurrentIndex(iterations->findData(item->data(0, Qt::UserRole)));
    }
  });
  firstButton = new QPushButton("|<", this);
  previousButton = new QPushButton("<", this);
  nextButton = new QPushButton(">", this);
  lastButton = new QPushButton(">|", this);

  firstButton->setToolTip("First completed iteration");
  previousButton->setToolTip("Previous completed iteration");
  nextButton->setToolTip("Next completed iteration");
  lastButton->setToolTip("Last completed iteration");

  QHBoxLayout *navigation = new QHBoxLayout();
  navigation->addWidget(firstButton);
  navigation->addWidget(previousButton);
  navigation->addStretch();
  navigation->addWidget(nextButton);
  navigation->addWidget(lastButton);
  layout->addLayout(navigation);

  autoPreview = new QCheckBox("Automatically preview iteration changes", this);
  layout->addWidget(autoPreview);
  iterationSlider = new QSlider(Qt::Horizontal, this);
  playButton = new QPushButton("Play", this);
  QHBoxLayout *playbackRow = new QHBoxLayout();
  playbackRow->addWidget(iterationSlider, 1);
  playbackRow->addWidget(playButton);
  layout->addLayout(playbackRow);
  playTimer = new QTimer(this);
  playTimer->setSingleShot(true);
  previewTimer = new QTimer(this);
  previewTimer->setSingleShot(true);
  QObject::connect(playButton, &QPushButton::clicked, this,
    &TopologyManagement::togglePlayback);
  QObject::connect(playTimer, &QTimer::timeout, this,
    &TopologyManagement::playbackFrame);
  QObject::connect(iterationSlider, &QSlider::valueChanged, this, [this](int index)
  {
    iterations->setCurrentIndex(index);
  });
  QObject::connect(autoPreview, &QCheckBox::toggled, this, [this](bool enabled)
  {
    if (enabled)
    {
      requestPreview();
    }
    else
    {
      previewTimer->stop();
    }
  });
  QObject::connect(previewTimer, &QTimer::timeout, this, [this]()
  {
    if (preview_busy)
    {
      previewTimer->start(75);
      return;
    }
    if (isVisible() && tabs->currentIndex() == IterationsTab && previewButton->isEnabled())
    {
      preview();
    }
  });


  metrics = new QLabel(this);
  metrics->setTextFormat(Qt::PlainText);
  metrics->setWordWrap(true);
  layout->addWidget(metrics);

  previewButton = new QPushButton("Preview", this);

  QHBoxLayout *previewRow = new QHBoxLayout();
  previewRow->addWidget(new QLabel("Threshold", tab));
  previewRow->addWidget(threshold);
  previewRow->addWidget(previewButton);
  layout->addLayout(previewRow);
  deleteElementsButton = new QPushButton("Delete below threshold", this);
  deleteElementsButton->setToolTip("Delete elements below the displayed threshold. Preserved elements are excluded.");
  layout->addWidget(deleteElementsButton);

  monitorButton = new QPushButton("Physical job monitor", this);
  QHBoxLayout *jobRow = new QHBoxLayout();
  jobRow->addWidget(monitorButton);
  layout->addLayout(jobRow);


  adjointMonitorButton = new QPushButton("Adjoint job monitor", this);
  jobRow->insertWidget(1, adjointMonitorButton);
  connect(adjointMonitorButton, &QPushButton::clicked, this, [this]() { openAdjointJob(); });

  layout->addStretch();
}

void TopologyManagement::connectIterationControls()
{
  QObject::connect(iterations, QOverload<int>::of(&QComboBox::currentIndexChanged),
    this, [this](int)
  {
    if (!playback_change)
    {
      pausePlayback();
    }
    updateDisplay();
    if (autoPreview->isChecked() && !playback_change)
    {
      requestPreview();
    }
  });
  QObject::connect(firstButton, &QPushButton::clicked, this, [this]()
  {
    if (iterations->count() > 0)
    {
      iterations->setCurrentIndex(0);
    }
  });
  QObject::connect(previousButton, &QPushButton::clicked, this, [this]()
  {
    const int index = iterations->currentIndex();

    if (index > 0)
    {
      iterations->setCurrentIndex(index - 1);
    }
  });
  QObject::connect(nextButton, &QPushButton::clicked, this, [this]()
  {
    const int index = iterations->currentIndex();

    if (index >= 0 && index + 1 < iterations->count())
    {
      iterations->setCurrentIndex(index + 1);
    }
  });
  QObject::connect(lastButton, &QPushButton::clicked, this, [this]()
  {
    if (iterations->count() > 0)
    {
      iterations->setCurrentIndex(iterations->count() - 1);
    }
  });
  QObject::connect(previewButton, &QPushButton::clicked, this, [this]()
  {
    preview();
  });

  connect(threshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value)
  {
    setResultThreshold(value);
  });
}

void TopologyManagement::updateDisplay()
{
  updateAnimationControls();
  synchronizeResultControls();
  adjointMonitorButton->setEnabled(false);
  if (animation_was_active)
  {
    return;
  }
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  const int index = iterations->currentIndex();
  if (index >= 0)
  {
    const QSignalBlocker blocker(iterationTree);
    for (int i = 0; i < iterationTree->topLevelItemCount(); ++i)
    {
      auto* item = iterationTree->topLevelItem(i);
      if (item->data(0, Qt::UserRole) == iterations->currentData())
      {
        iterationTree->setCurrentItem(item);
        break;
      }
    }
  }
  const bool selected = index >= 0;
  {
    const QSignalBlocker blocker(iterationSlider);
    iterationSlider->setRange(0, std::max(0, iterations->count() - 1));
    iterationSlider->setValue(std::max(0, index));
    iterationSlider->setEnabled(selected && iterations->count() > 1);
  }
  playButton->setEnabled(false);

  firstButton->setEnabled(selected && index > 0);
  previousButton->setEnabled(selected && index > 0);
  nextButton->setEnabled(selected && index + 1 < iterations->count());
  lastButton->setEnabled(selected && index + 1 < iterations->count());
  previewButton->setEnabled(false);
  threshold->setEnabled(false);
  deleteElementsButton->setEnabled(false);
  monitorButton->setEnabled(false);
  const bool canExport = currentTopologyId > 0 && !rows.empty() &&
    ccx_topology_iface.get_topology_run_state(currentTopologyId) != "running" &&
    meshAvailable(currentTopologyId, rows.begin()->first);
  gifButton->setEnabled(canExport);
  gifThreshold->setEnabled(canExport);
  wrapThreshold->setEnabled(false);
  wrapIterations->setEnabled(!rows.empty());
  wrapButton->setEnabled(false);

  if (!selected)
  {
    pausePlayback();
    metrics->setText(iterations->count() == 0
      ? "No completed iterations available."
      : "The requested iteration is not available.");
    return;
  }

  const int iteration_id = iterations->currentData().toInt();
  wrapPath->setPlaceholderText(QString("Optional: topo_%1_iteration_%2_wrap.stl").arg(loaded_case).arg(iteration_id));
  const auto entry = rows.find(iteration_id);

  if (entry == rows.end())
  {
    pausePlayback();
    metrics->setText("Iteration data is not available.");
    return;
  }

  const auto& row = entry->second;
  const auto format = [](const std::string& value) -> QString
  {
    if (value.empty())
    {
      return "N/A";
    }

    const QString text = QString::fromStdString(value);
    bool valid = false;
    const double number = text.toDouble(&valid);

    if (valid && std::isfinite(number))
    {
      return QString::number(number, 'g', 8);
    }

    return text;
  };

  QString convergence = "Not checked";

  if (row[Converged] == "1")
  {
    convergence = "Yes";
  }
  else if (row[Converged] == "0")
  {
    convergence = "No";
  }

  metrics->setText(QString(
    "Job: %1\n"
    "Objective: %2\n"
    "Volume fraction: %3\n"
    "Maximum density change: %4\n"
    "Relative objective change: %5\n"
    "Converged: %6")
    .arg(QString::fromStdString(row[JobId]))
    .arg(format(row[Objective]))
    .arg(format(row[VolumeFraction]))
    .arg(format(row[DensityChange]))
    .arg(format(row[ObjectiveChange]))
    .arg(convergence));
  metrics->setText(metrics->text() + "\n" + topologyDisplacementText(row));

  const std::string state = ccx_topology_iface.get_topology_run_state(loaded_case);
  bool valid = false;
  const int job_id = QString::fromStdString(row[JobId]).toInt(&valid);
  const bool has_job = !state.empty() && valid && job_id > 0 &&
    ccx_topology_iface.get_topology_iteration_job_id(loaded_case, iteration_id) == job_id &&
    ccx_jobs_iface.get_job_status(job_id) == 2;
  const bool can_use_mesh = has_job && state != "running" &&
    meshAvailable(loaded_case, iteration_id);

  monitorButton->setEnabled(has_job);
  const bool hasAdjoint = has_job && hasAdjointJob(iteration_id);
  adjointMonitorButton->setEnabled(hasAdjoint);
  previewButton->setEnabled(can_use_mesh);
  wrapButton->setEnabled(can_use_mesh);
  deleteElementsButton->setEnabled(can_use_mesh);
  threshold->setEnabled(can_use_mesh);
  wrapThreshold->setEnabled(can_use_mesh);
  playButton->setEnabled(can_use_mesh && iterations->count() > 1);
  if (!can_use_mesh)
  {
    pausePlayback();
  }

  if (has_job && state != "running" && !can_use_mesh)
  {
    metrics->setText(metrics->text() +
      "\nMesh operations are unavailable: stored element data "
      "is unavailable or some elements no longer exist.");
  }
}

void TopologyManagement::preview()
{
  CalculiXDrawInterface drawing;
  if (drawing.topology_animation_active())
  {
    pausePlayback();
    return;
  }
  if (preview_busy)
  {
    return;
  }
  const QScopedValueRollback<bool> guard(preview_busy, true);
  project(true);
}

void TopologyManagement::pausePlayback()
{
  playing = false;
  playTimer->stop();
  previewTimer->stop();
  playButton->setText("Play");
}

void TopologyManagement::requestPreview()
{
  CalculiXDrawInterface drawing;
  if (drawing.topology_animation_active())
  {
    return;
  }
  if (!playing && isVisible() && tabs->currentIndex() == IterationsTab && previewButton->isEnabled())
  {
    previewTimer->start(75);
  }
}

void TopologyManagement::togglePlayback()
{
  CalculiXDrawInterface drawing;
  if (drawing.topology_animation_active())
  {
    pausePlayback();
    return;
  }
  if (playing)
  {
    pausePlayback();
    return;
  }
  if (preview_busy || !previewButton->isEnabled() || iterations->count() < 2)
  {
    return;
  }
  previewTimer->stop();
  playing = true;
  playButton->setText("Pause");
  if (iterations->currentIndex() == iterations->count() - 1)
  {
    const QScopedValueRollback<bool> guard(playback_change, true);
    iterations->setCurrentIndex(0);
  }
  preview();
  if (playing)
  {
    playTimer->start(1000 / fps->value());
  }
}

void TopologyManagement::playbackFrame()
{
  CalculiXDrawInterface drawing;
  if (drawing.topology_animation_active())
  {
    pausePlayback();
    return;
  }
  if (!playing || !isVisible() || tabs->currentIndex() != IterationsTab)
  {
    pausePlayback();
    return;
  }
  if (preview_busy)
  {
    playTimer->start(20);
    return;
  }
  const int next = iterations->currentIndex() + 1;
  if (next >= iterations->count())
  {
    pausePlayback();
    return;
  }
  {
    const QScopedValueRollback<bool> guard(playback_change, true);
    iterations->setCurrentIndex(next);
  }
  if (!previewButton->isEnabled())
  {
    pausePlayback();
    return;
  }
  preview();
  if (playing)
  {
    playTimer->start(1000 / fps->value());
  }
}

bool TopologyManagement::selectedResult(
  int& topology_id, int& iteration_id, int& job_id, bool require_idle)
{
  CalculiXJobsInterface ccx_jobs_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  topology_id = -1;
  iteration_id = -1;
  job_id = -1;

  if (currentTopologyId <= 0 || iterations->currentIndex() < 0 ||
      loaded_case != currentTopologyId)
  {
    return false;
  }

  topology_id = loaded_case;
  iteration_id = iterations->currentData().toInt();
  const auto entry = rows.find(iteration_id);

  if (entry == rows.end())
  {
    return false;
  }

  const std::string state = ccx_topology_iface.get_topology_run_state(topology_id);

  if (state.empty())
  {
    return false;
  }
  CalculiXDrawInterface drawing;
  if (require_idle && drawing.topology_animation_active())
  {
    PRINT_ERROR("Finish or cancel the topology GIF export before using mesh actions.\n");
    return false;
  }
  if (require_idle && state == "running")
  {
    PRINT_ERROR("This action is unavailable while topology is running.\n");
    return false;
  }

  bool valid = false;
  job_id = QString::fromStdString(entry->second[JobId]).toInt(&valid);

  if (!valid || job_id <= 0 ||
      ccx_topology_iface.get_topology_iteration_job_id(topology_id, iteration_id) != job_id ||
      ccx_jobs_iface.get_job_status(job_id) != 2)
  {
    PRINT_ERROR("The displayed topology result is no longer current. "
      "Refresh Topology Management.\n");
    return false;
  }

  return true;
}

bool TopologyManagement::meshAvailable(int topology_id, int iteration_id)
{
  CalculiXTopologyInterface ccx_topology_iface;

  const auto elements =
    ccx_topology_iface.get_topology_iteration_element_data(topology_id, iteration_id);

  if (elements.empty())
  {
    return false;
  }

  for (const auto& element : elements)
  {
    if (element.size() != 4 || !std::isfinite(element[0]) ||
        element[0] < 1.0 || element[0] > std::numeric_limits<int>::max() ||
        std::floor(element[0]) != element[0])
    {
      return false;
    }
    if (!CubitInterface::get_element_exists(static_cast<int>(element[0])))
    {
      return false;
    }
  }

  return true;
}

void TopologyManagement::project(bool preview_only)
{
  CalculiXApplicationInterface ccx_application_iface;

  int topology_id;
  int iteration_id;
  int job_id;

  if (!selectedResult(topology_id, iteration_id, job_id, true))
  {
    pausePlayback();
    return;
  }
  if (!meshAvailable(topology_id, iteration_id))
  {
    PRINT_ERROR("The topology iteration cannot be applied to the current mesh.\n");
    updateDisplay();
    return;
  }

  QString command =
    QString("ccx result project topology %1 iteration %2 threshold %3")
      .arg(topology_id).arg(iteration_id)
      .arg(QString::number(threshold->value(), 'f', 6));

  if (preview_only)
  {
    command += " preview";
  }

  if (preview_only)
  {
    if (!ccx_application_iface.silent_cmd(command.toStdString()))
    {
      pausePlayback();
      PRINT_ERROR("Unable to preview the topology iteration.\n");
    }
  }
  else
  {
    pausePlayback();
    submitCommand(command);
  }

  if (!preview_only)
  {
    // Deletion can change the mesh even when a later batch fails.
    // Recheck availability without assuming command success.
    refreshIterations(iteration_id);
  }
}

void TopologyManagement::openJob()
{
  int topology_id;
  int iteration_id;
  int job_id;

  if (!selectedResult(topology_id, iteration_id, job_id))
  {
    return;
  }

  if (jobsMonitor != nullptr)
  {
    jobsMonitor->setJob(job_id);
    jobsMonitor->show();
    jobsMonitor->raise();
    jobsMonitor->update();
  }
}

