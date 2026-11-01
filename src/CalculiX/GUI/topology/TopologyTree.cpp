#include "TopologyTree.hpp"
#include <string>
#include <QString>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "GUI/jobs/JobsMonitor.hpp"
#include "GUI/application/ModelTree.hpp"
#include "TopologySettingsPanel.hpp"
#include "TopologyPreservedSetsPanel.hpp"
#include "TopologyResultsPanel.hpp"
#include "TopologyActionsPanel.hpp"
#include "CommandButtonPanel.hpp"
#include "CubitMessage.hpp"

#include <QAction>
#include <QApplication>
#include <QFont>
#include <QMenu>
#include <QSet>
#include <QStyle>
#include <QTreeWidget>
#include <QTimer>

namespace
{
std::string topology_job_icon(int status)
{
  switch (status)
  {
    case -1:
      return "TopologyTree_no_process";
    case 1:
    case 5:
      return "TopologyTree_running";
    case 2:
      return "TopologyTree_finished";
    case 3:
      return "TopologyTree_killed";
    case 4:
      return "TopologyTree_error";
    default:
      return "TopologyTree";
  }
}
}

TopologyTree::TopologyTree(QTreeWidget *parent)
  : QObject(parent), QTreeWidgetItem(parent)
{
}

TopologyTree::~TopologyTree()
{
}

void TopologyTree::initialize()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;

  setText(0, "Topology");
  setData(0, KindRole, Root);
  setIcon(0, ccx_gui_resources_iface.getIcon2("Topology"));
  if (refreshTimer == nullptr)
  {
    refreshTimer = new QTimer(this);
    QObject::connect(refreshTimer, &QTimer::timeout, this, [this]()
    {
      if (treeWidget() != nullptr && treeWidget()->isVisible())
      {
        update();
      }
    });
    refreshTimer->start(500);
  }
}

QTreeWidgetItem *TopologyTree::ensure(
  QTreeWidgetItem *parent, Kind kind, int id, const QString &label)
{
  QTreeWidgetItem *item = nullptr;
  for (int i = 0; i < parent->childCount(); ++i)
  {
    QTreeWidgetItem *candidate = parent->child(i);
    if (candidate->data(0, KindRole).toInt() == kind && candidate->data(0, IdRole).toInt() == id)
    {
      item = candidate;
      break;
    }
  }
  if (item == nullptr)
  {
    item = new QTreeWidgetItem(parent);
    item->setData(0, KindRole, kind);
    item->setData(0, IdRole, id);
  }
  item->setText(0, label);
  return item;
}

void TopologyTree::clear()
{
  while (childCount() > 0)
  {
    delete takeChild(0);
  }
}

void TopologyTree::update()
{
  CalculiXGuiResourcesInterface ccx_gui_resources_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  QSet<int> case_ids;
  for (const auto &row : ccx_topology_iface.get_topology_tree_data())
  {
    if (row.size() < 2)
    {
      continue;
    }
    bool valid = false;
    const int topology_id = QString::fromStdString(row[0]).toInt(&valid);
    if (!valid || topology_id <= 0)
    {
      continue;
    }
    case_ids.insert(topology_id);
    QTreeWidgetItem *case_item = ensure(this, Case, topology_id, QString::fromStdString(row[1]));
    case_item->setText(1, QString::number(topology_id));
    case_item->setData(0, TopologyRole, topology_id);
    const QString state = QString::fromStdString(ccx_topology_iface.get_topology_run_state(topology_id));
    case_item->setToolTip(0, QString("Topology %1: %2").arg(topology_id).arg(state));
    std::string icon_name = "TopologyTree_no_process";
    if (state == "running")
    {
      icon_name = "TopologyTree_running";
    }
    else if (state == "converged")
    {
      icon_name = "TopologyTree_finished";
    }
    else if (state == "iteration_limit")
    {
      icon_name = "TopologyTree_finished";
    }
    else if (state == "killed")
    {
      icon_name = "TopologyTree_killed";
    }
    else if (state == "failed")
    {
      icon_name = "TopologyTree_error";
    }
    case_item->setIcon(0, ccx_gui_resources_iface.getIcon2(icon_name));

    QTreeWidgetItem *iterations = ensure(case_item, Iterations, 0, "Iterations");
    QSet<int> iteration_ids;
    for (const auto &iteration : ccx_topology_iface.get_topology_iteration_data(topology_id))
    {
      if (iteration.size() < 9)
      {
        continue;
      }
      const int iteration_id = QString::fromStdString(iteration[0]).toInt(&valid);
      if (!valid || iteration_id < 1)
      {
        continue;
      }
      const int job_id = QString::fromStdString(iteration[1]).toInt(&valid);
      if (!valid || job_id <= 0)
      {
        continue;
      }
      iteration_ids.insert(iteration_id);
      const bool completed = iteration[2] == "2" && !iteration[3].empty();
      QString label = QString("Iteration %1").arg(iteration_id);
      QTreeWidgetItem *item = ensure(iterations, Iteration, iteration_id, label);
      item->setText(1, QString::number(iteration_id));
      item->setData(0, TopologyRole, topology_id);
      item->setData(0, IterationRole, iteration_id);
      item->setData(0, JobRole, job_id);
      item->setData(0, CompletedRole, completed);
      const QIcon icon = ccx_gui_resources_iface.getIcon2(
        topology_job_icon(QString::fromStdString(iteration[2]).toInt()));
      item->setIcon(0, icon);
      item->setToolTip(0, QString(
        "Objective: %1\nVolume fraction: %2\nMaximum density change: %3\nRelative objective change: %4")
        .arg(QString::fromStdString(iteration[3]), QString::fromStdString(iteration[4]),
             QString::fromStdString(iteration[6]), QString::fromStdString(iteration[7])));

      QTreeWidgetItem *job = ensure(item, Job, job_id, QString("Job %1").arg(job_id));
      job->setText(1, QString::number(job_id));
      job->setData(0, JobRole, job_id);
      job->setIcon(0, icon);
      for (int i = item->childCount() - 1; i >= 0; --i)
      {
        if (item->child(i) != job)
        {
          delete item->takeChild(i);
        }
      }
    }
    for (int i = iterations->childCount() - 1; i >= 0; --i)
    {
      if (!iteration_ids.contains(iterations->child(i)->data(0, IdRole).toInt()))
      {
        delete iterations->takeChild(i);
      }
    }
  }
  for (int i = childCount() - 1; i >= 0; --i)
  {
    if (!case_ids.contains(child(i)->data(0, IdRole).toInt()))
    {
      delete takeChild(i);
    }
  }
}

void TopologyTree::preview(QTreeWidgetItem *item)
{
  CalculiXApplicationInterface ccx_application_iface;

  if (item == nullptr || !item->data(0, CompletedRole).toBool())
  {
    return;
  }
  const QString command = QString("ccx result project topology %1 iteration %2 threshold 0.5 preview")
    .arg(item->data(0, TopologyRole).toInt()).arg(item->data(0, IterationRole).toInt());
  if (!ccx_application_iface.silent_cmd(command.toStdString()))
  {
    PRINT_ERROR("Topology preview failed.\n");
  }
}

void TopologyTree::open_job(int job_id, ModelTree &host)
{
  if (job_id <= 0 || host.myJobsMonitor == nullptr)
  {
    return;
  }
  host.myJobsMonitor->setJob(job_id);
  host.myJobsMonitor->show();
  host.myJobsMonitor->raise();
  host.myJobsMonitor->update();
}

void TopologyTree::open_actions(int topology_id, ModelTree &host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXTopologyActions");
  for (QWidget *widget : host.cmdpanel->findChildren<QWidget *>())
  {
    auto *panel = dynamic_cast<TopologyActionsPanel *>(widget);
    if (panel != nullptr)
    {
      panel->setTopology(topology_id);
      return;
    }
  }
  PRINT_ERROR("Unable to open topology actions.\n");
}

void TopologyTree::open_results(int topology_id, int iteration_id, ModelTree &host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXTopologyResults");

  for (QWidget *widget : host.cmdpanel->findChildren<QWidget *>())
  {
    auto *panel = dynamic_cast<TopologyResultsPanel *>(widget);

    if (panel != nullptr)
    {
      panel->setIteration(topology_id, iteration_id);
      return;
    }
  }

  PRINT_ERROR("Unable to open the topology results panel.\n");
}

void TopologyTree::open_preserved_sets(int topology_id, ModelTree &host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }

  host.setWidgetInCmdPanelMarker("CCXTopologyPreservedSets");

  for (QWidget *widget : host.cmdpanel->findChildren<QWidget *>())
  {
    auto *panel = dynamic_cast<TopologyPreservedSetsPanel *>(widget);

    if (panel != nullptr)
    {
      panel->setTopology(topology_id);
      return;
    }
  }

  PRINT_ERROR("Unable to open topology preservation management.\n");
}

void TopologyTree::open_settings(int topology_id, ModelTree &host)
{
  if (host.cmdpanel == nullptr)
  {
    return;
  }
  host.setWidgetInCmdPanelMarker("CCXTopologyModify");
  for (QWidget *widget : host.cmdpanel->findChildren<QWidget *>())
  {
    TopologySettingsPanel *panel = dynamic_cast<TopologySettingsPanel *>(widget);
    if (panel != nullptr && panel->isModify())
    {
      panel->setTopology(topology_id);
      return;
    }
  }
  PRINT_ERROR("Unable to open the topology settings panel.\n");
}

void TopologyTree::activate(QTreeWidgetItem *item, ModelTree &host)
{
  if (item == nullptr)
  {
    return;
  }
  const int kind = item->data(0, KindRole).toInt();
  if (kind == Case)
  {
    open_settings(item->data(0, TopologyRole).toInt(), host);
  }
  else if (kind == Root)
  {
    host.setWidgetInCmdPanelMarker("CCXTopologyCreate");
  }
  else if (kind == Iteration)
  {
    preview(item);
  }
  else if (kind == Job)
  {
    open_job(item->data(0, JobRole).toInt(), host);
  }
}

void TopologyTree::populateContextMenu(QMenu &menu, QTreeWidgetItem *item, ModelTree &host)
{
  CalculiXTopologyInterface ccx_topology_iface;

  if (item == nullptr)
  {
    return;
  }
  const int kind = item->data(0, KindRole).toInt();
  const int topology_id = item->data(0, TopologyRole).toInt();
  if (kind == Root)
  {
    addPanelAction(menu, "Create topology", host, "CCXTopologyCreate");
  }
  else if (kind == Case)
  {
    const bool running = ccx_topology_iface.get_topology_run_state(topology_id) == "running";
    QAction *actions = menu.addAction("Actions");
    QObject::connect(actions, &QAction::triggered, &menu, [topology_id, &host]()
    {
      open_actions(topology_id, host);
    });
    QAction *preservation = menu.addAction("Manage preserved sets");
    QObject::connect(preservation, &QAction::triggered, &menu, [topology_id, &host]()
    {
      open_preserved_sets(topology_id, host);
    });
    QAction *modify = menu.addAction("Modify settings");
    modify->setEnabled(!running);
    QObject::connect(modify, &QAction::triggered, &menu, [topology_id, &host]()
    {
      open_settings(topology_id, host);
    });
    addCommandAction(menu, "Run", QString("ccx run topology %1").arg(topology_id));
    menu.actions().last()->setEnabled(!running);
    addCommandAction(menu, "Kill", QString("ccx kill topology %1").arg(topology_id));
    menu.actions().last()->setEnabled(running);
    addCommandAction(menu, "Delete", QString("ccx delete topology %1").arg(topology_id));
    menu.actions().last()->setEnabled(!running);
  }
  else if (kind == Iteration || kind == Job)
  {
    const int job_id = item->data(0, JobRole).toInt();
    if (kind == Iteration)
    {
      const int iteration_id = item->data(0, IterationRole).toInt();
      const bool completed = item->data(0, CompletedRole).toBool();
      QAction *browse = menu.addAction("Browse iteration");
      browse->setEnabled(completed);
      QObject::connect(browse, &QAction::triggered, &menu,
        [topology_id, iteration_id, &host]()
      {
        open_results(topology_id, iteration_id, host);
      });
      QAction *previewAction = menu.addAction("Preview (threshold 0.5)");
      previewAction->setEnabled(completed && ccx_topology_iface.get_topology_run_state(topology_id) != "running");
      QObject::connect(previewAction, &QAction::triggered, &menu, [this, topology_id, iteration_id]()
      {
        CalculiXApplicationInterface ccx_application_iface;

        const QString command = QString("ccx result project topology %1 iteration %2 threshold 0.5 preview")
          .arg(topology_id).arg(iteration_id);
        if (!ccx_application_iface.silent_cmd(command.toStdString()))
        {
          PRINT_ERROR("Topology preview failed.\n");
        }
      });
      addCommandAction(menu, "ParaView", QString("ccx result paraview job %1").arg(job_id));
      menu.actions().last()->setEnabled(completed);
    }
    QAction *monitor = menu.addAction("Job monitor");
    QObject::connect(monitor, &QAction::triggered, &menu, [job_id, &host]()
    {
      open_job(job_id, host);
    });
  }
}
