#include <type_traits>
#include "TopologyActionsPanel.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include <string>
#include <vector>
#include <array>
#include <QString>
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "TopologyDisplacementText.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "GUI/jobs/JobsMonitor.hpp"
#include "CubitMessage.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QTimer>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLocale>
#include <QProgressBar>
#include <cmath>

TopologyActionsPanel::TopologyActionsPanel(QWidget *parent) : QWidget(parent)
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

  status = new QLabel(this);
  status->setTextFormat(Qt::PlainText);
  status->setWordWrap(true);
  layout->addWidget(status);
  wait = new QCheckBox("Wait until finished", this);
  layout->addWidget(wait);

  QLabel *note = new QLabel(
    "Starting a run resets this case's previous iteration jobs and results.", this);
  note->setWordWrap(true);
  layout->addWidget(note);

  runButton = new QPushButton("Run", this);
  waitButton = new QPushButton("Wait", this);
  killButton = new QPushButton("Kill", this);
  QHBoxLayout *buttons = new QHBoxLayout();
  buttons->addWidget(runButton);
  buttons->addWidget(waitButton);
  buttons->addWidget(killButton);
  layout->addLayout(buttons);
  monitorButton = new QPushButton("Job monitor", this);
  layout->addWidget(monitorButton);
  createConvergenceDisplay(layout);
  auto *measurementGroup = new QGroupBox("Displacement monitoring", this);
  auto *measurementLayout = new QVBoxLayout(measurementGroup);
  displacementMeasurement = new QLabel("No completed iteration", measurementGroup);
  displacementMeasurement->setTextFormat(Qt::PlainText);
  displacementMeasurement->setWordWrap(true);
  measurementLayout->addWidget(displacementMeasurement);
  layout->addWidget(measurementGroup);
  layout->addStretch();

  QObject::connect(cases, QOverload<int>::of(&QComboBox::currentIndexChanged),
    this, [this](int)
  {
    updateControls();
  });
  QObject::connect(reload, &QPushButton::clicked, this, [this]()
  {
    refreshCases();
  });
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
  QObject::connect(monitorButton, &QPushButton::clicked, this, [this]()
  {
    openJob();
  });
  QTimer *poll = new QTimer(this);
  QObject::connect(poll, &QTimer::timeout, this, [this]()
  {
    if (isVisible())
    {
      updateControls();
    }
  });
  poll->start(500);
  updateControls();
}

void TopologyActionsPanel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshCases();
}

void TopologyActionsPanel::setTopology(int topology_id)
{
  refreshCases(topology_id);
}

void TopologyActionsPanel::refreshCases(int preferred_id)
{
  CalculiXTopologyInterface ccx_topology_iface;

  const bool explicit_case = preferred_id > 0;
  if (!explicit_case)
  {
    preferred_id = cases->currentData().toInt();
  }
  {
    const QSignalBlocker blocker(cases);
    cases->clear();
    for (const auto& row : ccx_topology_iface.get_topology_tree_data())
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
    const int index = cases->findData(preferred_id);
    if (index >= 0 || explicit_case)
    {
      cases->setCurrentIndex(index);
    }
  }
  updateControls();
}

void TopologyActionsPanel::updateControls()
{
  CalculiXTopologyInterface ccx_topology_iface;

  if (cases->currentIndex() >= 0)
  {
    const std::string selected_id = std::to_string(cases->currentData().toInt());
    bool selected_exists = false;

    for (const auto& row : ccx_topology_iface.get_topology_tree_data())
    {
      if (row.size() >= 2 && row[0] == selected_id)
      {
        selected_exists = true;
        break;
      }
    }

    if (!selected_exists)
    {
      refreshCases();
      return;
    }
  }

  updateConvergence();
  runButton->setEnabled(false);
  waitButton->setEnabled(false);
  killButton->setEnabled(false);
  monitorButton->setEnabled(false);
  wait->setEnabled(false);
  if (cases->currentIndex() < 0)
  {
    status->setText("No topology case selected.");
    return;
  }
  TopologyRunInfo information;
  if (!ccx_topology_iface.get_topology_run_info(cases->currentData().toInt(), information))
  {
    status->setText("Topology information is unavailable. Reload the case list.");
    return;
  }
  status->setText(QString("State: %1\nIteration: %2\nJob: %3")
    .arg([&]() -> std::common_type_t<decltype(QString("Stopping")), decltype(QString::fromStdString(information.state))> {
      if (information.stop_requested && information.active) {
        return QString("Stopping");
      } else {
        return QString::fromStdString(information.state);
      }
    }())
    .arg([&]() -> std::common_type_t<decltype(QString::number(information.iteration_id)), decltype("-")> {
      if (information.iteration_id > 0) {
        return QString::number(information.iteration_id);
      } else {
        return "-";
      }
    }())
    .arg([&]() -> std::common_type_t<decltype(QString::number(information.job_id)), decltype("-")> {
      if (information.job_id > 0) {
        return QString::number(information.job_id);
      } else {
        return "-";
      }
    }()));
  runButton->setEnabled(!information.active);
  waitButton->setEnabled(information.active && !information.waiting);
  killButton->setEnabled(information.active && !information.stop_requested);
  monitorButton->setEnabled(information.job_id > 0);
  wait->setEnabled(!information.active);
}

void TopologyActionsPanel::execute(const QString& action)
{
  CalculiXTopologyInterface ccx_topology_iface;

  if (cases->currentIndex() < 0)
  {
    return;
  }
  const int topology_id = cases->currentData().toInt();
  TopologyRunInfo information;
  if (!ccx_topology_iface.get_topology_run_info(topology_id, information))
  {
    refreshCases();
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
  GuiSendCmd::gui_cmd(command);
  updateControls();
}

void TopologyActionsPanel::openJob()
{
  CalculiXTopologyInterface ccx_topology_iface;

  TopologyRunInfo information;
  if (cases->currentIndex() < 0 ||
      !ccx_topology_iface.get_topology_run_info(cases->currentData().toInt(), information) ||
      information.job_id <= 0)
  {
    return;
  }
  for (QWidget *widget : QApplication::topLevelWidgets())
  {
    auto *monitor = dynamic_cast<JobsMonitor *>(widget);
    if (monitor != nullptr)
    {
      monitor->setJob(information.job_id);
      monitor->show();
      monitor->raise();
      monitor->update();
      return;
    }
  }
  PRINT_ERROR("The job monitor is not available.\n");
}

void TopologyActionsPanel::createConvergenceDisplay(QVBoxLayout *layout)
{
  auto *group = new QGroupBox("Convergence", this);
  auto *content = new QVBoxLayout(group);
  content->setSpacing(6);
  group->setToolTip("Latest completed iteration. Criteria use current case settings. Bars show limit usage; lower is better.");

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
    heading->addWidget(new QLabel(names[i], group));
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

int TopologyActionsPanel::updateCriterion(int index, const QString& text, double limit)
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

  const QString multiplier = [&]() -> QString {
    if (std::isfinite(ratio)) {
      return locale.toString(ratio, 'f', 2) + "x limit";
    } else {
      return QString("above limit");
    }
  }();
  const QString color = [&]() -> QString {
    if (passed) {
      return "#2e7d32";
    } else {
      return "#b26a00";
    }
  }();
  const QString background = [&]() -> QString {
    if (passed) {
      return "#e8f5e9";
    } else {
      return "#fff3e0";
    }
  }();

  criterion.bar->setValue(barValue);
  criterion.bar->setFormat(QString("%1 / %2 (%3)").arg(locale.toString(value, 'g', 8)).arg(locale.toString(limit, 'g', 8)).arg(multiplier));
  criterion.bar->setStyleSheet(QString("QProgressBar { border: 1px solid %1; border-radius: 3px; text-align: center; background: palette(base); } QProgressBar::chunk { background: %2; }").arg(color).arg([&]() -> const char* {
    if (passed) {
      return "#81c784";
    } else {
      return "#ffb74d";
    }
  }()));
  criterion.badge->setText([&]() -> const char* {
    if (passed) {
      return "PASS";
    } else {
      return "WAIT";
    }
  }());
  criterion.badge->setStyleSheet(QString("color: %1; background: %2; border-radius: 3px; padding: 2px 5px; font-weight: bold;").arg(color).arg(background));

  if (passed) {
    return 1;
  } else {
    return 0;
  }
}

void TopologyActionsPanel::updateConvergence()
{
  CalculiXTopologyInterface ccx_topology_iface;

  displacementMeasurement->setText("No completed iteration");
  for (int i = 0; i < static_cast<int>(criteria.size()); ++i)
  {
    updateCriterion(i, QString(), 1.0);
  }

  convergenceIteration->setText("No completed iteration");
  convergenceSummary->setText("Converged: not checked");
  convergenceSummary->setStyleSheet("");

  if (cases->currentIndex() < 0)
  {
    return;
  }

  const int topology_id = cases->currentData().toInt();
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

  const int density = updateCriterion(0, QString::fromStdString((*latest)[6]), settings.density_tolerance);
  const int compliance = updateCriterion(1, QString::fromStdString((*latest)[7]), settings.compliance_tolerance);

  // Match the existing CoreTopology volume convergence allowance.
  constexpr double volumeTolerance = 1.0e-6;
  const int volume = updateCriterion(2, QString::fromStdString((*latest)[4]), settings.volume_fraction + volumeTolerance);
  const int passed = ([&]() -> int {
    if (density == 1) {
      return 1;
    } else {
      return 0;
    }
  }()) + ([&]() -> int {
    if (compliance == 1) {
      return 1;
    } else {
      return 0;
    }
  }()) + ([&]() -> int {
    if (volume == 1) {
      return 1;
    } else {
      return 0;
    }
  }());

  if (density < 0 || compliance < 0 || volume < 0 || (*latest)[8].empty())
  {
    convergenceSummary->setText(QString("Converged: not checked - %1 / 3 displayed criteria met").arg(passed));
    return;
  }

  const bool converged = (*latest)[8] == "1";

  if (!converged && passed == 3 && (settings.objective == TopologyObjective::Compliance || settings.objective == TopologyObjective::Displacement))
  {
    convergenceSummary->setText("Converged: NO - ordinary density proposal has not settled");
  }
  else
  {
    convergenceSummary->setText(QString("Converged: %1 - %2 / 3 displayed criteria met").arg([&]() -> const char* {
      if (converged) {
        return "YES";
      } else {
        return "NO";
      }
    }()).arg(passed));
  }

  convergenceSummary->setStyleSheet([&]() -> const char* {
    if (converged) {
      return "color: #2e7d32; font-weight: bold;";
    } else {
      return "color: #b26a00; font-weight: bold;";
    }
  }());
}

void TopologyActionsPanel::clearTopology(int topology_id)
{
  if (topology_id > 0 && cases->currentData().toInt() != topology_id)
  {
    return;
  }
  const QSignalBlocker blocker(cases);
  cases->clear();
  updateControls();
}
