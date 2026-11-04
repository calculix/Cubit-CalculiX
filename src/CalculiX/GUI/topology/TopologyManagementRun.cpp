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

#include <QLocale>
#include <QProgressBar>
void TopologyManagement::createRunTab(QWidget* tab)
{
  auto* layout = new QVBoxLayout(tab);
  status = new QLabel(this);
  status->setTextFormat(Qt::PlainText);
  status->setWordWrap(true);
  wait = new QCheckBox("Wait until finished", this);

  QLabel *note = new QLabel(
    "Starting a run resets this case's previous iteration jobs and results.", this);
  note->setWordWrap(true);

  runButton = new QPushButton("Run", this);
  waitButton = new QPushButton("Wait", this);
  killButton = new QPushButton("Kill", this);
  QHBoxLayout *buttons = new QHBoxLayout();
  buttons->addWidget(runButton);
  buttons->addWidget(waitButton);
  buttons->addWidget(killButton);
  runMonitorButton = new QPushButton("Job monitor", this);
  createConvergenceDisplay(layout);
  auto *measurementGroup = new QGroupBox("Displacement monitoring", this);
  auto *measurementLayout = new QVBoxLayout(measurementGroup);
  displacementMeasurement = new QLabel("No completed iteration", measurementGroup);
  displacementMeasurement->setTextFormat(Qt::PlainText);
  displacementMeasurement->setWordWrap(true);
  measurementLayout->addWidget(displacementMeasurement);
  layout->addWidget(measurementGroup);
  layout->addStretch();
  layout->addWidget(status);
  layout->addWidget(wait);
  layout->addWidget(note);
  layout->addLayout(buttons);
  layout->addWidget(runMonitorButton);

  QObject::connect(runButton, &QPushButton::clicked, this, [this]()
  {
    execute("run");
  });
  QObject::connect(waitButton, &QPushButton::clicked, this, [this]()
  {
    execute("wait");
  });
  QObject::connect(killButton, &QPushButton::clicked, this, [this]()
  {
    execute("kill");
  });
  QObject::connect(runMonitorButton, &QPushButton::clicked, this, [this]()
  {
    openActiveJob();
  });
}

void TopologyManagement::updateControls()
{
  CalculiXTopologyInterface ccx_topology_iface;

  updateConvergence();
  runButton->setEnabled(false);
  waitButton->setEnabled(false);
  killButton->setEnabled(false);
  runMonitorButton->setEnabled(false);
  wait->setEnabled(false);
  if (currentTopologyId <= 0 || creating)
  {
    status->setText("No topology case selected.");
    return;
  }
  TopologyRunInfo information;
  if (!ccx_topology_iface.get_topology_run_info(currentTopologyId, information))
  {
    status->setText("Topology information is unavailable. Reload the case list.");
    return;
  }
  status->setText(QString("State: %1\nIteration: %2\nJob: %3")
    .arg(information.stop_requested && information.active
      ? QString("Stopping") : QString::fromStdString(information.state))
    .arg(information.iteration_id > 0 ? QString::number(information.iteration_id) : "-")
    .arg(information.job_id > 0 ? QString::number(information.job_id) : "-"));
  TopologyOptions options;
  const bool runnable = ccx_topology_iface.get_topology_settings(currentTopologyId, options) && options.method == TopologyMethod::Density;
  runButton->setEnabled(runnable && !information.active && !animation_was_active);
  runButton->setToolTip(runnable ? "Run this topology case." : "Level-set execution remains blocked pending verification.");
  waitButton->setEnabled(information.active && !information.waiting);
  killButton->setEnabled(information.active && !information.stop_requested);
  runMonitorButton->setEnabled(information.job_id > 0);
  wait->setEnabled(!information.active);
}

void TopologyManagement::execute(const QString& action)
{
  CalculiXTopologyInterface ccx_topology_iface;

  if (currentTopologyId <= 0 || creating)
  {
    return;
  }
  const int topology_id = currentTopologyId;
  TopologyRunInfo information;
  if (!ccx_topology_iface.get_topology_run_info(topology_id, information))
  {
    refresh();
    return;
  }
  if ((action == "run" && information.active) ||
      (action != "run" && !information.active))
  {
    updateControls();
    return;
  }
  QString command = QString("ccx %1 topology %2").arg(action).arg(topology_id);
  if (action == "run" && wait->isChecked())
  {
    command += " wait";
  }
  submitCommand(command);
  updateControls();
}

void TopologyManagement::openActiveJob()
{
  CalculiXTopologyInterface ccx_topology_iface;

  TopologyRunInfo information;
  if (currentTopologyId <= 0 || creating ||
      !ccx_topology_iface.get_topology_run_info(currentTopologyId, information) ||
      information.job_id <= 0)
  {
    return;
  }
  if (jobsMonitor != nullptr)
  {
    jobsMonitor->setJob(information.job_id);
    jobsMonitor->show();
    jobsMonitor->raise();
    jobsMonitor->update();
  }
}

void TopologyManagement::createConvergenceDisplay(QVBoxLayout *layout)
{
  auto *group = new QGroupBox("Convergence", this);
  auto *content = new QVBoxLayout(group);
  content->setSpacing(6);
  group->setToolTip("Latest completed iteration. Density and objective limits use current case settings; level-set field tolerance is captured with its design. Bars show limit usage; lower is better.");

  convergenceIteration = new QLabel("No completed iteration", group);
  content->addWidget(convergenceIteration);

  const std::array<QString, 3> names = {
    "Maximum density change",
    "Relative objective change",
    "Volume fraction"
  };

  for (std::size_t i = 0; i < criteria.size(); ++i)
  {
    auto& criterion = criteria[i];
    auto *heading = new QHBoxLayout();
    criterion.label = new QLabel(names[i], group);
    heading->addWidget(criterion.label);
    heading->addStretch();

    criterion.badge = new QLabel("-", group);
    criterion.badge->setAlignment(Qt::AlignCenter);
    criterion.badge->setMinimumWidth(48);
    heading->addWidget(criterion.badge);
    content->addLayout(heading);

    criterion.bar = new QProgressBar(group);
    criterion.bar->setRange(0, 10000);
    criterion.bar->setValue(0);
    criterion.bar->setFormat("Not available");
    criterion.bar->setFixedHeight(18);
    criterion.bar->setToolTip("Measured value / allowed limit. A full amber bar means the limit is exceeded.");
    content->addWidget(criterion.bar);
  }

  convergenceSummary = new QLabel("Converged: not checked", group);
  content->addWidget(convergenceSummary);
  layout->addWidget(group);
}

int TopologyManagement::updateCriterion(int index, const QString& text, double limit)
{
  auto& criterion = criteria[index];
  bool valid = false;
  const double value = text.toDouble(&valid);

  if (!valid || !std::isfinite(value) || value < 0.0 || !std::isfinite(limit) || limit <= 0.0)
  {
    criterion.bar->setValue(0);
    criterion.bar->setFormat("Not available");
    criterion.bar->setStyleSheet("");
    criterion.badge->setText("-");
    criterion.badge->setStyleSheet("");
    return -1;
  }

  const bool passed = value <= limit;
  const double ratio = value / limit;
  const QLocale locale;
  int barValue = 10000;

  if (passed)
  {
    barValue = static_cast<int>(std::round(ratio * 10000.0));
  }

  const QString multiplier = std::isfinite(ratio) ? locale.toString(ratio, 'f', 2) + "x limit" : QString("above limit");
  const QString color = passed ? "#2e7d32" : "#b26a00";
  const QString background = passed ? "#e8f5e9" : "#fff3e0";

  criterion.bar->setValue(barValue);
  criterion.bar->setFormat(QString("%1 / %2 (%3)").arg(locale.toString(value, 'g', 8)).arg(locale.toString(limit, 'g', 8)).arg(multiplier));
  criterion.bar->setStyleSheet(QString("QProgressBar { border: 1px solid %1; border-radius: 3px; text-align: center; background: palette(base); } QProgressBar::chunk { background: %2; }").arg(color).arg(passed ? "#81c784" : "#ffb74d"));
  criterion.badge->setText(passed ? "PASS" : "WAIT");
  criterion.badge->setStyleSheet(QString("color: %1; background: %2; border-radius: 3px; padding: 2px 5px; font-weight: bold;").arg(color).arg(background));

  return passed ? 1 : 0;
}

void TopologyManagement::updateConvergence()
{
  CalculiXTopologyInterface ccx_topology_iface;

  displacementMeasurement->setText("No completed iteration");
  for (int i = 0; i < static_cast<int>(criteria.size()); ++i)
  {
    updateCriterion(i, QString(), 1.0);
  }

  criteria[0].label->setText("Maximum design change");
  convergenceIteration->setText("No completed iteration");
  convergenceSummary->setText("Converged: not checked");
  convergenceSummary->setStyleSheet("");

  if (currentTopologyId <= 0 || creating)
  {
    return;
  }

  const int topology_id = currentTopologyId;
  TopologyOptions settings;

  if (!ccx_topology_iface.get_topology_settings(topology_id, settings))
  {
    return;
  }

  const auto rows = ccx_topology_iface.get_topology_iteration_data(topology_id);
  const std::vector<std::string> *latest = nullptr;
  int latestIteration = -1;

  for (const auto& row : rows)
  {
    if (row.size() < 9 || row[2] != "2" || row[3].empty())
    {
      continue;
    }

    bool valid = false;
    const int iteration_id = QString::fromStdString(row[0]).toInt(&valid);

    if (valid && iteration_id > latestIteration)
    {
      latest = &row;
      latestIteration = iteration_id;
    }
  }

  if (latest == nullptr)
  {
    return;
  }

  convergenceIteration->setText(QString("Completed iteration %1").arg(latestIteration));
  displacementMeasurement->setText(topologyDisplacementText(*latest));

  TopologyMethod method;
  if (!ccx_topology_iface.get_topology_iteration_method(topology_id, latestIteration, method))
  {
    return;
  }
  double changeTolerance = settings.density_tolerance;
  if (method == TopologyMethod::LevelSet)
  {
    TopologyLevelSetSettings captured;
    if (!ccx_topology_iface.get_topology_iteration_level_set_settings(topology_id, latestIteration, captured))
    {
      return;
    }
    criteria[0].label->setText("Maximum field change");
    changeTolerance = captured.field_tolerance;
  }
  else
  {
    criteria[0].label->setText("Maximum density change");
  }
  const int density = updateCriterion(0, QString::fromStdString((*latest)[6]), changeTolerance);
  const int compliance = updateCriterion(1, QString::fromStdString((*latest)[7]), settings.compliance_tolerance);

  // Match the existing CoreTopology volume convergence allowance.
  constexpr double volumeTolerance = 1.0e-6;
  const int volume = updateCriterion(2, QString::fromStdString((*latest)[4]), settings.volume_fraction + volumeTolerance);
  const int passed = (density == 1 ? 1 : 0) + (compliance == 1 ? 1 : 0) + (volume == 1 ? 1 : 0);

  if (density < 0 || compliance < 0 || volume < 0 || (*latest)[8].empty())
  {
    convergenceSummary->setText(QString("Converged: not checked - %1 / 3 displayed criteria met").arg(passed));
    return;
  }

  const bool converged = (*latest)[8] == "1";

  if (!converged && passed == 3)
  {
    convergenceSummary->setText("Converged: NO - additional Core convergence checks did not pass");
  }
  else
  {
    convergenceSummary->setText(QString("Converged: %1 - %2 / 3 displayed criteria met").arg(converged ? "YES" : "NO").arg(passed));
  }

  convergenceSummary->setStyleSheet(converged ? "color: #2e7d32; font-weight: bold;" : "color: #b26a00; font-weight: bold;");
}
