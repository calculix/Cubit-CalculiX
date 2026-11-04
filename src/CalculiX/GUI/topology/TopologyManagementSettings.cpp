#include "TopologyManagement.hpp"
#include "TopologyManagementDensitySettingsWidget.hpp"
#include "TopologyManagementLevelSetSettingsWidget.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include <QComboBox>
#include <QFormLayout>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStackedWidget>
#include <QTabWidget>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <limits>

void TopologyManagement::createSettingsTab(QWidget* tab)
{
  auto* layout = new QVBoxLayout(tab);
  settingsForm = new QWidget(tab);
  auto* content = new QVBoxLayout(settingsForm);
  auto* selection = new QFormLayout();
  settingsMethod = new QComboBox(settingsForm);
  settingsMethod->addItem("Density-based", "density");
  settingsMethod->addItem("Level set", "level_set");
  settingsMethod->setToolTip("Choose the topology method to configure. Apply stores a method change; level-set execution remains blocked pending verification.");
  selection->addRow("Method", settingsMethod);
  content->addLayout(selection);
  settingsStack = new QStackedWidget(settingsForm);
  const auto submit = [this](const QString& command)
  {
    return !animation_was_active && submitCommand(command);
  };
  const auto applied = [this](int topology_id)
  {
    settingsMethodDrafts.erase(creating ? 0 : currentTopologyId);
    creating = false;
    currentTopologyId = topology_id;
    settingsMethodDrafts.erase(topology_id);
    refresh();
    loadSettings();
    refreshLists();
  };
  densitySettings = new TopologyManagementDensitySettingsWidget(submit, applied, settingsStack);
  levelSetSettings = new TopologyManagementLevelSetSettingsWidget(submit, applied, settingsStack);
  settingsStack->addWidget(densitySettings);
  settingsStack->addWidget(levelSetSettings);
  content->addWidget(settingsStack, 1);
  layout->addWidget(settingsForm, 1);
  connect(settingsMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index)
  {
    const int new_id = settingsStack->currentIndex() == 0 ? densitySettings->topologyId() : levelSetSettings->topologyId();
    rememberSettingsDraft();
    settingsStack->setCurrentIndex(index);
    if (index == 0)
    {
      densitySettings->setContext(currentTopologyId, creating, new_id);
    }
    else
    {
      levelSetSettings->setContext(currentTopologyId, creating, new_id);
    }
    settingsMethodDrafts[creating ? 0 : currentTopologyId] = index;
  });
}

void TopologyManagement::rememberSettingsDraft()
{
  densitySettings->rememberDraft();
  levelSetSettings->rememberDraft();
}

void TopologyManagement::loadSettings()
{
  int method = 0;
  if (!creating && currentTopologyId > 0)
  {
    TopologyOptions options;
    if (!CalculiXTopologyInterface().get_topology_settings(currentTopologyId, options))
    {
      settingsForm->setEnabled(false);
      return;
    }
    method = options.method == TopologyMethod::LevelSet ? 1 : 0;
  }
  const auto draft = settingsMethodDrafts.find(creating ? 0 : currentTopologyId);
  if (draft != settingsMethodDrafts.end())
  {
    method = draft->second;
  }
  int next_id = 1;
  for (int i = 0; i < caseTree->topLevelItemCount(); ++i)
  {
    const int id = caseTree->topLevelItem(i)->data(0, Qt::UserRole).toInt();
    if (id >= next_id && id < (std::numeric_limits<int>::max)())
    {
      next_id = id + 1;
    }
  }
  const QSignalBlocker blocker(settingsMethod);
  settingsMethod->setCurrentIndex(method);
  settingsStack->setCurrentIndex(method);
  densitySettings->setContext(currentTopologyId, creating, next_id);
  levelSetSettings->setContext(currentTopologyId, creating, next_id);
  settingsLoadedId = currentTopologyId;
  const bool running = !creating && currentTopologyId > 0 && CalculiXTopologyInterface().get_topology_run_state(currentTopologyId) == "running";
  settingsForm->setEnabled((creating || currentTopologyId > 0) && !running && !animation_was_active);
}

void TopologyManagement::beginCreate()
{
  if (submitting || animation_was_active)
  {
    return;
  }
  rememberSettingsDraft();
  pausePlayback();
  creating = true;
  currentTopologyId = -1;
  loadSettings();
  refresh();
  tabs->setCurrentIndex(SettingsTab);
}

void TopologyManagement::deleteTopology()
{
  if (submitting || creating || currentTopologyId <= 0 || animation_was_active)
  {
    return;
  }
  if (CalculiXTopologyInterface().get_topology_run_state(currentTopologyId) == "running")
  {
    return;
  }
  submitCommand(QString("ccx delete topology %1").arg(currentTopologyId));
  refresh();
  loadSettings();
  refreshLists();
}
