#include "TopologyResultsPanel.hpp"
#include "CubitMessage.hpp"
#include "GuiSendCmd.hpp"
#include "JobsMonitor.hpp"
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

TopologyResultsPanel::TopologyResultsPanel(QWidget *parent)
  : QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);

  cases = new QComboBox(this);
  QPushButton *reload = new QPushButton("Reload", this);

  QHBoxLayout *caseRow = new QHBoxLayout();
  caseRow->addWidget(cases, 1);
  caseRow->addWidget(reload);

  QFormLayout *form = new QFormLayout();
  form->addRow("Topology case", caseRow);
  layout->addLayout(form);

  firstButton = new QPushButton("|<", this);
  previousButton = new QPushButton("<", this);
  nextButton = new QPushButton(">", this);
  lastButton = new QPushButton(">|", this);
  iterations = new QComboBox(this);

  firstButton->setToolTip("First completed iteration");
  previousButton->setToolTip("Previous completed iteration");
  nextButton->setToolTip("Next completed iteration");
  lastButton->setToolTip("Last completed iteration");

  QHBoxLayout *navigation = new QHBoxLayout();
  navigation->addWidget(firstButton);
  navigation->addWidget(previousButton);
  navigation->addWidget(iterations, 1);
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
    &TopologyResultsPanel::togglePlayback);
  QObject::connect(playTimer, &QTimer::timeout, this,
    &TopologyResultsPanel::playbackFrame);
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
    if (isVisible() && previewButton->isEnabled())
    {
      preview();
    }
  });


  metrics = new QLabel(this);
  metrics->setTextFormat(Qt::PlainText);
  metrics->setWordWrap(true);
  layout->addWidget(metrics);

  threshold = new QDoubleSpinBox(this);
  threshold->setRange(0.0, 1.0);
  threshold->setDecimals(6);
  threshold->setSingleStep(0.05);
  threshold->setValue(0.5);

  previewButton = new QPushButton("Preview", this);

  QHBoxLayout *previewRow = new QHBoxLayout();
  previewRow->addWidget(new QLabel("Threshold", this));
  previewRow->addWidget(threshold);
  previewRow->addWidget(previewButton);
  layout->addLayout(previewRow);
  deleteElementsButton = new QPushButton("Delete below threshold", this);
  deleteElementsButton->setToolTip(
    "Delete elements below the displayed threshold. Preserved elements are excluded.");
  layout->addWidget(deleteElementsButton);

  monitorButton = new QPushButton("Job monitor", this);
  paraviewButton = new QPushButton("ParaView", this);
  QHBoxLayout *jobRow = new QHBoxLayout();
  jobRow->addWidget(monitorButton);
  jobRow->addWidget(paraviewButton);
  layout->addLayout(jobRow);

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
  gifButton->setToolTip(
    "Export all completed iterations of this case using the displayed threshold.");
  layout->addWidget(gifButton);

  QObject::connect(deleteElementsButton, &QPushButton::clicked, this, [this]()
  {
    project(false);
  });
  QObject::connect(monitorButton, &QPushButton::clicked, this, [this]()
  {
    openJob();
  });
  QObject::connect(paraviewButton, &QPushButton::clicked, this, [this]()
  {
    openParaView();
  });
  QObject::connect(browse, &QPushButton::clicked, this, [this]()
  {
    browseGif();
  });
  QObject::connect(gifButton, &QPushButton::clicked, this, [this]()
  {
    exportGif();
  });
  QGroupBox *wrapGroup = new QGroupBox("Alpha wrapping", this);
  QFormLayout *wrapForm = new QFormLayout(wrapGroup);

  wrapAlpha = new QLineEdit(wrapGroup);
  wrapOffset = new QLineEdit(wrapGroup);

  wrapAlpha->setPlaceholderText("Optional: calculated automatically");
  wrapOffset->setPlaceholderText("Optional: calculated automatically");

  wrapAlpha->setToolTip("Leave empty for bounding-box diagonal / 20. Otherwise enter a positive value in model length units.");
  wrapOffset->setToolTip("Leave empty for resolved alpha / 30. Otherwise enter a positive value in model length units.");

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

  QObject::connect(wrapBrowse, &QPushButton::clicked, this, &TopologyResultsPanel::browseWrap);
  QObject::connect(wrapButton, &QPushButton::clicked, this, &TopologyResultsPanel::wrapIteration);

  layout->addStretch();

  QObject::connect(cases, QOverload<int>::of(&QComboBox::currentIndexChanged),
    this, [this](int)
  {
    refreshIterations();
  });
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
  QObject::connect(reload, &QPushButton::clicked, this, [this]()
  {
    refreshCases();
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

  QObject::connect(threshold, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this, [this](double)
  {
    if (autoPreview->isChecked())
    {
      requestPreview();
    }
  });
  updateDisplay();
}

void TopologyResultsPanel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshCases();
}

void TopologyResultsPanel::hideEvent(QHideEvent *event)
{
  pausePlayback();
  QWidget::hideEvent(event);
}

void TopologyResultsPanel::setIteration(int topology_id, int iteration_id)
{
  refreshCases(topology_id, iteration_id);
}

void TopologyResultsPanel::refreshCases(int preferred_case, int preferred_iteration)
{
  const bool explicit_case = preferred_case > 0;

  if (!explicit_case)
  {
    preferred_case = cases->currentData().toInt();
  }

  {
    const QSignalBlocker blocker(cases);
    cases->clear();

    for (const auto& row : ccx_iface.get_topology_tree_data())
    {
      if (row.size() < 2)
      {
        continue;
      }

      bool valid = false;
      const int topology_id = QString::fromStdString(row[0]).toInt(&valid);

      if (valid && topology_id > 0)
      {
        cases->addItem(QString("%1: %2").arg(topology_id)
          .arg(QString::fromStdString(row[1])), topology_id);
      }
    }

    const int index = cases->findData(preferred_case);

    if (index >= 0 || explicit_case)
    {
      cases->setCurrentIndex(index);
    }
  }

  refreshIterations(preferred_iteration);
}

void TopologyResultsPanel::refreshIterations(int preferred_iteration)
{
  pausePlayback();
  const bool explicit_iteration = preferred_iteration >= 0;
  const int topology_id = cases->currentData().toInt();

  if (!explicit_iteration && loaded_case == topology_id &&
      iterations->currentIndex() >= 0)
  {
    preferred_iteration = iterations->currentData().toInt();
  }

  {
    const QSignalBlocker blocker(iterations);
    iterations->clear();
    rows.clear();
    loaded_case = -1;

    if (cases->currentIndex() >= 0)
    {
      for (const auto& row : ccx_iface.get_topology_iteration_data(topology_id))
      {
        if (row.size() < ColumnCount || row[JobStatus] != "2" || row[Objective].empty())
        {
          continue;
        }

        bool valid = false;
        const int iteration_id = QString::fromStdString(row[IterationId]).toInt(&valid);

        if (!valid || iteration_id < 1)
        {
          continue;
        }

        const int job_id = QString::fromStdString(row[JobId]).toInt(&valid);

        if (!valid || job_id <= 0)
        {
          continue;
        }

        rows.emplace(iteration_id, row);
      }

      // std::map keeps actual iteration IDs in numerical order.
      for (const auto& entry : rows)
      {
        iterations->addItem(QString("Iteration %1").arg(entry.first), entry.first);
      }

      loaded_case = topology_id;
      const int index = iterations->findData(preferred_iteration);

      if (index >= 0 || explicit_iteration)
      {
        iterations->setCurrentIndex(index);
      }
      else
      {
        // Opening a case initially shows its last completed iteration.
        iterations->setCurrentIndex(iterations->count() - 1);
      }
    }
  }

  updateDisplay();
  if (autoPreview->isChecked())
  {
    requestPreview();
  }
}

void TopologyResultsPanel::updateDisplay()
{
  const int index = iterations->currentIndex();
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
  paraviewButton->setEnabled(false);
  gifButton->setEnabled(false);
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

  const std::string state = ccx_iface.get_topology_run_state(loaded_case);
  bool valid = false;
  const int job_id = QString::fromStdString(row[JobId]).toInt(&valid);
  const bool has_job = !state.empty() && valid && job_id > 0 &&
    ccx_iface.get_topology_iteration_job_id(loaded_case, iteration_id) == job_id &&
    ccx_iface.get_job_status(job_id) == 2;
  const bool can_use_mesh = has_job && state != "running" &&
    meshAvailable(loaded_case, iteration_id);

  monitorButton->setEnabled(has_job);
  paraviewButton->setEnabled(has_job);
  previewButton->setEnabled(can_use_mesh);
  wrapButton->setEnabled(can_use_mesh);
  deleteElementsButton->setEnabled(can_use_mesh);
  threshold->setEnabled(can_use_mesh);
  gifButton->setEnabled(can_use_mesh);
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

void TopologyResultsPanel::preview()
{
  if (preview_busy)
  {
    return;
  }
  const QScopedValueRollback<bool> guard(preview_busy, true);
  project(true);
}

void TopologyResultsPanel::pausePlayback()
{
  playing = false;
  playTimer->stop();
  previewTimer->stop();
  playButton->setText("Play");
}

void TopologyResultsPanel::requestPreview()
{
  if (!playing && isVisible() && previewButton->isEnabled())
  {
    previewTimer->start(75);
  }
}

void TopologyResultsPanel::togglePlayback()
{
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

void TopologyResultsPanel::playbackFrame()
{
  if (!playing || !isVisible())
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

bool TopologyResultsPanel::selectedResult(
  int& topology_id, int& iteration_id, int& job_id, bool require_idle)
{
  topology_id = -1;
  iteration_id = -1;
  job_id = -1;

  if (cases->currentIndex() < 0 || iterations->currentIndex() < 0 ||
      loaded_case != cases->currentData().toInt())
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

  const std::string state = ccx_iface.get_topology_run_state(topology_id);

  if (state.empty())
  {
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
      ccx_iface.get_topology_iteration_job_id(topology_id, iteration_id) != job_id ||
      ccx_iface.get_job_status(job_id) != 2)
  {
    PRINT_ERROR("The displayed topology result is no longer current. "
      "Reload the results panel.\n");
    return false;
  }

  return true;
}

bool TopologyResultsPanel::meshAvailable(int topology_id, int iteration_id)
{
  const auto elements =
    ccx_iface.get_topology_iteration_element_data(topology_id, iteration_id);

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

void TopologyResultsPanel::project(bool preview_only)
{
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
    if (!ccx_iface.silent_cmd(command.toStdString()))
    {
      pausePlayback();
      PRINT_ERROR("Unable to preview the topology iteration.\n");
    }
  }
  else
  {
    pausePlayback();
    GuiSendCmd::gui_cmd(command);
  }

  if (!preview_only)
  {
    // Deletion can change the mesh even when a later batch fails.
    // Recheck availability without assuming command success.
    refreshIterations(iteration_id);
  }
}

void TopologyResultsPanel::openJob()
{
  int topology_id;
  int iteration_id;
  int job_id;

  if (!selectedResult(topology_id, iteration_id, job_id))
  {
    return;
  }

  // Reuse the monitor already created by ModelTree.
  for (QWidget *widget : QApplication::topLevelWidgets())
  {
    auto *monitor = dynamic_cast<JobsMonitor *>(widget);

    if (monitor != nullptr)
    {
      monitor->setJob(job_id);
      monitor->show();
      monitor->raise();
      monitor->update();
      return;
    }
  }

  PRINT_ERROR("The job monitor is not available.\n");
}

void TopologyResultsPanel::openParaView()
{
  int topology_id;
  int iteration_id;
  int job_id;

  if (!selectedResult(topology_id, iteration_id, job_id))
  {
    return;
  }

  GuiSendCmd::gui_cmd(QString("ccx result paraview job %1").arg(job_id));
}

void TopologyResultsPanel::browseGif()
{
  const QString filename = QFileDialog::getSaveFileName(this,
    "Export topology animation", gifPath->text(), "GIF animation (*.gif)",
    nullptr, QFileDialog::DontConfirmOverwrite);

  if (!filename.isEmpty())
  {
    gifPath->setText(QDir::fromNativeSeparators(filename));
  }
}

void TopologyResultsPanel::exportGif()
{
  int topology_id;
  int iteration_id;
  int job_id;

  if (!selectedResult(topology_id, iteration_id, job_id, true))
  {
    return;
  }
  if (!meshAvailable(topology_id, iteration_id))
  {
    PRINT_ERROR("Topology animation is unavailable for the current mesh.\n");
    updateDisplay();
    return;
  }

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

  GuiSendCmd::gui_cmd(command);
}

void TopologyResultsPanel::browseWrap()
{
  QString suggested = wrapPath->text();

  if (suggested.trimmed().isEmpty() && cases->currentIndex() >= 0 && iterations->currentIndex() >= 0)
  {
    suggested = QString("topo_%1_iteration_%2_wrap.stl").arg(cases->currentData().toInt()).arg(iterations->currentData().toInt());
  }

  const QString filename = QFileDialog::getSaveFileName(this, "Wrapped STL", suggested, "STL files (*.stl)", nullptr, QFileDialog::DontConfirmOverwrite);

  if (!filename.isEmpty())
  {
    wrapPath->setText(QDir::fromNativeSeparators(filename));
  }
}

void TopologyResultsPanel::wrapIteration()
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
  GuiSendCmd::gui_cmd(command);
  updateDisplay();
}
