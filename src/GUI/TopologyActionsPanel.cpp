#include "TopologyActionsPanel.hpp"
#include "GuiSendCmd.hpp"
#include "JobsMonitor.hpp"
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
  const bool explicit_case = preferred_id > 0;
  if (!explicit_case)
  {
    preferred_id = cases->currentData().toInt();
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
  if (!ccx_iface.get_topology_run_info(cases->currentData().toInt(), information))
  {
    status->setText("Topology information is unavailable. Reload the case list.");
    return;
  }
  status->setText(QString("State: %1\nIteration: %2\nJob: %3")
    .arg(information.stop_requested && information.active
      ? QString("Stopping") : QString::fromStdString(information.state))
    .arg(information.iteration_id > 0 ? QString::number(information.iteration_id) : "-")
    .arg(information.job_id > 0 ? QString::number(information.job_id) : "-"));
  runButton->setEnabled(!information.active);
  waitButton->setEnabled(information.active && !information.waiting);
  killButton->setEnabled(information.active && !information.stop_requested);
  monitorButton->setEnabled(information.job_id > 0);
  wait->setEnabled(!information.active);
}

void TopologyActionsPanel::execute(const QString& action)
{
  if (cases->currentIndex() < 0)
  {
    return;
  }
  const int topology_id = cases->currentData().toInt();
  TopologyRunInfo information;
  if (!ccx_iface.get_topology_run_info(topology_id, information))
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
  TopologyRunInfo information;
  if (cases->currentIndex() < 0 ||
      !ccx_iface.get_topology_run_info(cases->currentData().toInt(), information) ||
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
