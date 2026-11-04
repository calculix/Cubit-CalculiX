#include "TopologyManagement.hpp"
#include "TopologyManagementDensitySettingsWidget.hpp"
#include "TopologyManagementLevelSetSettingsWidget.hpp"
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

#include <QScrollBar>
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

#include <set>

TopologyManagement::TopologyManagement(JobsMonitor* monitor, QWidget* parent)
  : QWidget(parent, Qt::Window), jobsMonitor(monitor)
{
  setAttribute(Qt::WA_QuitOnClose, false);
  setWindowTitle("Topology Management");
  resize(1200, 720);
  if (auto* screen = QGuiApplication::primaryScreen())
  {
    const QSize available = screen->availableGeometry().size() - QSize(40, 80);
    resize(size().boundedTo(available));
  }
  auto* outer = new QVBoxLayout(this);
  auto* splitter = new QSplitter(this);
  auto* left = new QWidget(splitter);
  auto* leftLayout = new QVBoxLayout(left);
  leftLayout->setContentsMargins(0, 0, 0, 0);
  caseTree = new QTreeWidget(left);
  caseTree->setRootIsDecorated(false);
  caseTree->setHeaderLabels({"Name", "ID", "State"});
  caseTree->setColumnWidth(0, 160);
  caseTree->setColumnWidth(1, 40);
  leftLayout->addWidget(caseTree);
  newButton = new QPushButton("New", left);
  deleteButton = new QPushButton("Delete", left);
  auto* reload = new QPushButton("Refresh", left);
  auto* caseButtons = new QHBoxLayout();
  caseButtons->addWidget(newButton);
  caseButtons->addWidget(deleteButton);
  caseButtons->addWidget(reload);
  leftLayout->addLayout(caseButtons);
  auto* right = new QWidget(splitter);
  auto* rightLayout = new QVBoxLayout(right);
  rightLayout->setContentsMargins(0, 0, 0, 0);
  heading = new QLabel(right);
  heading->setTextFormat(Qt::PlainText);
  rightLayout->addWidget(heading);
  tabs = new QTabWidget(right);
  rightLayout->addWidget(tabs, 1);
  const QStringList names = {"Settings", "Preserved sets", "Run", "Iterations", "GIF", "Alpha wrap"};
  for (const auto& name : names)
  {
    tabs->addTab(new QWidget(tabs), name);
  }
  createSettingsTab(tabs->widget(SettingsTab));
  createPreservationTab(tabs->widget(PreservationTab));
  createRunTab(tabs->widget(RunTab));
  createIterationsTab(tabs->widget(IterationsTab));
  createAnimationTab(tabs->widget(AnimationTab));
  createWrapTab(tabs->widget(WrapTab));
  connectIterationControls();
  splitter->setStretchFactor(0, 0);
  splitter->setStretchFactor(1, 1);
  splitter->setSizes({280, 880});
  outer->addWidget(splitter, 1);
  auto* bottom = new QHBoxLayout();
  bottom->addStretch();
  auto* closeButton = new QPushButton("Close", this);
  bottom->addWidget(closeButton);
  outer->addLayout(bottom);
  connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
  connect(newButton, &QPushButton::clicked, this, [this]() { beginCreate(); });
  connect(deleteButton, &QPushButton::clicked, this, [this]() { deleteTopology(); });
  connect(reload, &QPushButton::clicked, this, [this]() { refresh(); refreshLists(); });
  connect(caseTree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* item, QTreeWidgetItem*)
  {
    if (item != nullptr && !refreshing)
    {
      selectTopology(item->data(0, Qt::UserRole).toInt());
    }
  });
  connect(tabs, &QTabWidget::currentChanged, this, [this](int index)
  {
    if (index != IterationsTab)
    {
      pausePlayback();
    }
    if (index == PreservationTab)
    {
      refreshLists();
    }
  });
  refreshTimer = new QTimer(this);
  connect(refreshTimer, &QTimer::timeout, this, [this]()
  {
    if (isVisible())
    {
      refresh();
    }
  });
  refreshTimer->start(500);
  clear();
}

void TopologyManagement::clear(bool clear_settings)
{
  pausePlayback();
  currentTopologyId = -1;
  creating = false;
  if (clear_settings)
  {
    densitySettings->clearTopology();
    levelSetSettings->clearTopology();
    settingsMethodDrafts.clear();
  }
  settingsLoadedId = -2;
  iterationRecords.clear();
  lastRunState.clear();
  caseTree->clear();
  iterationTree->clear();
  iterations->clear();
  rows.clear();
  wrapIterations->clear();
  gifPath->clear();
  wrapPath->clear();
  wrapAlpha->clear();
  wrapOffset->clear();
  setResultThreshold(0.5);
  loaded_case = -1;
  loadSettings();
  refreshLists();
  updateControls();
  updateDisplay();
  heading->setText("Select a topology case or create a new case.");
  deleteButton->setEnabled(false);
}

void TopologyManagement::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  refresh();
}

void TopologyManagement::hideEvent(QHideEvent* event)
{
  pausePlayback();
  QWidget::hideEvent(event);
}

void TopologyManagement::closeEvent(QCloseEvent* event)
{
  pausePlayback();
  QWidget::closeEvent(event);
}

void TopologyManagement::setTopology(int topology_id)
{
  selectTopology(topology_id);
  tabs->setCurrentIndex(SettingsTab);
}

void TopologyManagement::setIteration(int topology_id, int iteration_id)
{
  selectTopology(topology_id);
  refreshIterations(iteration_id);
  tabs->setCurrentIndex(IterationsTab);
  if (autoPreview->isChecked())
  {
    requestPreview();
  }
}

void TopologyManagement::selectTopology(int topology_id)
{
  if (submitting || animation_was_active)
  {
    return;
  }
  rememberSettingsDraft();
  pausePlayback();
  creating = false;
  const bool changed = currentTopologyId != topology_id;
  currentTopologyId = topology_id;
  if (changed)
  {
    gifPath->clear();
    wrapPath->clear();
    iterationRecords.clear();
    lastRunState.clear();
    loaded_case = -1;
  }
  refresh();
  loadSettings();
  refreshLists();
}

void TopologyManagement::refresh()
{
  if (refreshing || submitting)
  {
    return;
  }
  const QScopedValueRollback<bool> guard(refreshing, true);
  CalculiXTopologyInterface core;
  const auto data = core.get_topology_tree_data();
  const QSignalBlocker blocker(caseTree);
  std::set<int> existing;
  for (const auto& row : data)
  {
    if (row.size() < 2)
    {
      continue;
    }
    bool valid = false;
    const int id = QString::fromStdString(row[0]).toInt(&valid);
    if (!valid || id <= 0)
    {
      continue;
    }
    existing.insert(id);
    QTreeWidgetItem* item = nullptr;
    for (int i = 0; i < caseTree->topLevelItemCount(); ++i)
    {
      if (caseTree->topLevelItem(i)->data(0, Qt::UserRole).toInt() == id)
      {
        item = caseTree->topLevelItem(i);
        break;
      }
    }
    if (item == nullptr)
    {
      item = new QTreeWidgetItem(caseTree);
      item->setData(0, Qt::UserRole, id);
    }
    item->setText(0, QString::fromStdString(row[1]));
    item->setText(1, QString::number(id));
    item->setText(2, QString::fromStdString(core.get_topology_run_state(id)));
  }
  for (int i = caseTree->topLevelItemCount() - 1; i >= 0; --i)
  {
    auto* item = caseTree->topLevelItem(i);
    if (existing.count(item->data(0, Qt::UserRole).toInt()) == 0)
    {
      const int removed = item->data(0, Qt::UserRole).toInt();
      densitySettings->clearTopology(removed);
      levelSetSettings->clearTopology(removed);
      settingsMethodDrafts.erase(removed);
      delete caseTree->takeTopLevelItem(i);
    }
  }
  if (!creating && existing.count(currentTopologyId) == 0)
  {
    currentTopologyId = caseTree->topLevelItemCount() > 0 ? caseTree->topLevelItem(0)->data(0, Qt::UserRole).toInt() : -1;
    loadSettings();
    refreshLists();
  }
  for (int i = 0; i < caseTree->topLevelItemCount(); ++i)
  {
    auto* item = caseTree->topLevelItem(i);
    if (!creating && item->data(0, Qt::UserRole).toInt() == currentTopologyId)
    {
      caseTree->setCurrentItem(item);
      heading->setText(QString("%1 — Topology %2    State: %3").arg(item->text(0)).arg(currentTopologyId).arg(item->text(2)));
    }
  }
  if (currentTopologyId <= 0 || creating)
  {
    iterationRecords.clear();
    refreshIterations();
    heading->setText(creating ? "New topology case" : "Select a topology case or create a new case.");
  }
  else
  {
    const auto records = core.get_topology_iteration_data(currentTopologyId);
    const auto state = core.get_topology_run_state(currentTopologyId);
    if (records != iterationRecords || loaded_case != currentTopologyId || state != lastRunState)
    {
      iterationRecords = records;
      lastRunState = state;
      refreshIterations();
    }
  }
  if (!creating && currentTopologyId > 0 && settingsLoadedId != currentTopologyId)
  {
    loadSettings();
  }
  updateControls();
  const bool running = currentTopologyId > 0 && core.get_topology_run_state(currentTopologyId) == "running";
  const bool settingsEditable = (creating || currentTopologyId > 0) && !running && !animation_was_active;
  settingsForm->setEnabled(settingsEditable);
  densitySettings->setEditable(settingsEditable);
  levelSetSettings->setEditable(settingsEditable);
  deleteButton->setEnabled(currentTopologyId > 0 && !running && !animation_was_active && !creating);
  newButton->setEnabled(!animation_was_active);
  // Re-enable preservation immediately after a run without clearing the user's selection.
  editable = currentTopologyId > 0 && !creating && !running && !animation_was_active;
  updateButtons();
}

void TopologyManagement::refreshIterations(int preferred_iteration)
{
  pausePlayback();
  const int previousCase = loaded_case;
  const int scrollPosition = iterationTree->verticalScrollBar()->value();
  const bool explicitIteration = preferred_iteration > 0;
  if (!explicitIteration && loaded_case == currentTopologyId)
  {
    preferred_iteration = iterations->currentData().toInt();
  }
  const QSignalBlocker comboBlocker(iterations);
  const QSignalBlocker treeBlocker(iterationTree);
  const QSignalBlocker wrapBlocker(wrapIterations);
  iterations->clear();
  wrapIterations->clear();
  iterationTree->clear();
  rows.clear();
  loaded_case = currentTopologyId;
  if (currentTopologyId > 0 && !creating)
  {
    CalculiXTopologyInterface core;
    iterationRecords = core.get_topology_iteration_data(currentTopologyId);
    for (const auto& row : iterationRecords)
    {
      if (row.size() < ColumnCount)
      {
        continue;
      }
      bool valid = false;
      const int id = QString::fromStdString(row[IterationId]).toInt(&valid);
      if (!valid || id < 1)
      {
        continue;
      }
      auto* item = new QTreeWidgetItem(iterationTree);
      item->setData(0, Qt::UserRole, id);
      item->setText(0, QString::number(id));
      item->setText(1, QString::fromStdString(row[JobId]));
      const int status = QString::fromStdString(row[JobStatus]).toInt();
      item->setText(2, status == 2 ? "Finished" : (status == 1 || status == 5 ? "Running" : (status == 3 ? "Killed" : (status == 4 ? "Failed" : "Not started"))));
      item->setText(3, QString::fromStdString(row[Objective]));
      item->setText(4, QString::fromStdString(row[VolumeFraction]));
      item->setText(5, row[Converged].empty() ? "—" : (row[Converged] == "1" ? "Yes" : "No"));
      if (status == 2 && !row[Objective].empty())
      {
        rows.emplace(id, row);
      }
    }
    for (const auto& entry : rows)
    {
      iterations->addItem(QString("Iteration %1").arg(entry.first), entry.first);
      wrapIterations->addItem(QString("Iteration %1").arg(entry.first), entry.first);
    }
    const int index = iterations->findData(preferred_iteration);
    iterations->setCurrentIndex(index >= 0 || explicitIteration ? index : iterations->count() - 1);
  }
  updateDisplay();
  if (!explicitIteration && previousCase == currentTopologyId)
  {
    iterationTree->verticalScrollBar()->setValue(scrollPosition);
  }
  if (autoPreview->isChecked())
  {
    requestPreview();
  }
}

void TopologyManagement::synchronizeResultControls()
{
  const QSignalBlocker iterationBlocker(wrapIterations);
  wrapIterations->setCurrentIndex(wrapIterations->findData(iterations->currentData()));
  const QSignalBlocker gifBlocker(gifThreshold);
  const QSignalBlocker wrapBlocker(wrapThreshold);
  gifThreshold->setValue(threshold->value());
  wrapThreshold->setValue(threshold->value());
}

void TopologyManagement::setResultThreshold(double value)
{
  const QSignalBlocker blocker(threshold);
  threshold->setValue(value);
  synchronizeResultControls();
  if (autoPreview->isChecked() && tabs->currentIndex() == IterationsTab)
  {
    requestPreview();
  }
}

bool TopologyManagement::hasAdjointJob(int iteration_id)
{
  // An adjoint is absent for compliance and a zero displacement objective.
  CalculiXTopologyInterface core;
  TopologyOptions options;
  const auto entry = rows.find(iteration_id);
  if (entry == rows.end() || !core.get_topology_settings(currentTopologyId, options) || options.objective != TopologyObjective::Displacement || QString::fromStdString(entry->second[Objective]).toDouble() == 0.0)
  {
    return false;
  }
  return core.get_topology_iteration_adjoint_job_id(currentTopologyId, iteration_id) > 0;
}

void TopologyManagement::openAdjointJob()
{
  if (jobsMonitor == nullptr || iterations->currentIndex() < 0 || !hasAdjointJob(iterations->currentData().toInt()))
  {
    return;
  }
  CalculiXTopologyInterface core;
  const int id = core.get_topology_iteration_adjoint_job_id(currentTopologyId, iterations->currentData().toInt());
  if (id > 0)
  {
    jobsMonitor->setJob(id);
    jobsMonitor->show();
    jobsMonitor->raise();
    jobsMonitor->update();
  }
}

bool TopologyManagement::submitCommand(const QString& command)
{
  if (submitting || refreshing)
  {
    return false;
  }
  // Native commands may process GUI events. Keep source selection fixed until return.
  const QScopedValueRollback<bool> guard(submitting, true);
  const bool wasEnabled = isEnabled();
  setEnabled(false);
  const bool submitted = GuiSendCmd::gui_cmd(command);
  setEnabled(wasEnabled);
  return submitted;
}

void TopologyManagement::invalidateTopology(int topology_id)
{
  densitySettings->clearTopology(topology_id);
  levelSetSettings->clearTopology(topology_id);
  settingsMethodDrafts.erase(topology_id);
  if (currentTopologyId != topology_id)
  {
    return;
  }
  // Deletion notifications can arrive in a batch. Clear without querying half-reset cores.
  const int selected = currentTopologyId;
  clear(false);
  currentTopologyId = selected;
}
