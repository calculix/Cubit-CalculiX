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


namespace
{
const QStringList settingKeys = {"name", "volume_fraction", "void_ratio", "iterations", "filter_radius", "move_limit", "density_tolerance", "compliance_tolerance", "material_cap", "method", "objective", "displacement_nodeset", "displacement_component", "adaptive_moves", "minimum_move_limit"};
}

void TopologyManagement::createSettingsTab(QWidget* tab)
{
  auto* layout = new QVBoxLayout(tab);
  auto* scroll = new QScrollArea(tab);
  scroll->setWidgetResizable(true);
  settingsForm = new QWidget(scroll);
  auto* form = new QFormLayout(settingsForm);
  settingsId = new QSpinBox(settingsForm);
  settingsId->setRange(1, std::numeric_limits<int>::max());
  form->addRow("Topology ID", settingsId);
  const QStringList labels = {"Name", "Volume fraction", "Void stiffness ratio", "Maximum iterations", "Filter radius", "Density move limit", "Density tolerance", "Relative objective tolerance", "Material cap"};
  for (std::size_t i = 0; i < settingsFields.size(); ++i)
  {
    settingsFields[i] = new QLineEdit(settingsForm);
    settingsFields[i]->setPlaceholderText("Optional: use core default");
    form->addRow(labels[static_cast<int>(i)], settingsFields[i]);
    connect(settingsFields[i], &QLineEdit::textEdited, this, [this]() { settingsDirty = true; });
  }
  settingsFields[7]->setToolTip("Relative objective change allowed for convergence; 0.001 corresponds to 0.1 percent.");
  settingsFields[8]->setToolTip("Maximum generated topology materials. Applies to new iterations; the core validates the value.");
  settingsMethod = new QComboBox(settingsForm);
  settingsMethod->addItem("Default", "");
  settingsMethod->addItem("Density", "density");
  settingsMethod->addItem("Level set (not implemented)", "level_set");
  if (auto* model = qobject_cast<QStandardItemModel*>(settingsMethod->model()))
  {
    model->item(2)->setEnabled(false);
  }
  settingsObjective = new QComboBox(settingsForm);
  settingsObjective->addItem("Default", "");
  settingsObjective->addItem("Compliance", "compliance");
  settingsObjective->addItem("Mass (not implemented)", "mass");
  settingsObjective->addItem("Displacement", "displacement");
  if (auto* model = qobject_cast<QStandardItemModel*>(settingsObjective->model()))
  {
    model->item(2)->setEnabled(false);
  }
  settingsNodeset = new QComboBox(settingsForm);
  settingsComponent = new QComboBox(settingsForm);
  settingsComponent->addItem("Default", "");
  settingsComponent->addItem("X", "x");
  settingsComponent->addItem("Y", "y");
  settingsComponent->addItem("Z", "z");
  settingsComponent->addItem("Magnitude", "magnitude");
  settingsAdaptive = new QComboBox(settingsForm);
  settingsAdaptive->addItem("Default", "");
  settingsAdaptive->addItem("On", "adaptive_moves_on");
  settingsAdaptive->addItem("Off", "adaptive_moves_off");
  settingsAdaptive->setToolTip("Adapt the density move limit for compliance and displacement objectives.");
  settingsMinimumMove = new QLineEdit(settingsForm);
  settingsMinimumMove->setPlaceholderText("Optional: use core default");
  form->addRow("Method", settingsMethod);
  form->addRow("Objective", settingsObjective);
  form->addRow("Displacement nodeset", settingsNodeset);
  form->addRow("Displacement component", settingsComponent);
  form->addRow("Adaptive density moves", settingsAdaptive);
  form->addRow("Minimum move limit", settingsMinimumMove);
  connect(settingsMinimumMove, &QLineEdit::textEdited, this, [this]() { settingsDirty = true; });
  for (auto* combo : {settingsMethod, settingsObjective, settingsNodeset, settingsComponent, settingsAdaptive})
  {
    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
      if (!loadingSettings)
      {
        settingsDirty = true;
      }
      settingsComponent->setEnabled(settingsNodeset->currentData().toInt() > 0);
    });
  }
  scroll->setWidget(settingsForm);
  layout->addWidget(scroll, 1);
  settingsMessage = new QLabel(tab);
  settingsMessage->setWordWrap(true);
  settingsMessage->setTextFormat(Qt::PlainText);
  layout->addWidget(settingsMessage);
  auto* buttons = new QHBoxLayout();
  auto* reload = new QPushButton("Reload settings", tab);
  settingsApply = new QPushButton("Apply", tab);
  buttons->addWidget(reload);
  buttons->addStretch();
  buttons->addWidget(settingsApply);
  layout->addLayout(buttons);
  connect(reload, &QPushButton::clicked, this, [this]()
  {
    settingsDrafts.erase(creating ? 0 : currentTopologyId);
    settingsDirty = false;
    loadSettings();
  });
  connect(settingsApply, &QPushButton::clicked, this, [this]() { applySettings(); });
}

QStringList TopologyManagement::settingsValues() const
{
  QStringList values;
  for (auto* field : settingsFields)
  {
    values.append(field->text().trimmed());
  }
  values << settingsMethod->currentData().toString() << settingsObjective->currentData().toString()
         << settingsNodeset->currentData().toString() << settingsComponent->currentData().toString()
         << settingsAdaptive->currentData().toString() << settingsMinimumMove->text().trimmed();
  return values;
}

QStringList TopologyManagement::settingsValues(const TopologyOptions& options) const
{
  const char* objective = topology_objective_name(options.objective);
  const QStringList components = {"magnitude", "x", "y", "z"};
  return {QString::fromStdString(options.name), QString::number(options.volume_fraction, 'g', 17),
    QString::number(options.void_ratio, 'g', 17), QString::number(options.maximum_iterations),
    QString::number(options.filter_radius, 'g', 17), QString::number(options.move_limit, 'g', 17),
    QString::number(options.density_tolerance, 'g', 17), QString::number(options.compliance_tolerance, 'g', 17),
    QString::number(options.material_cap), options.method == TopologyMethod::Density ? "density" : "level_set",
    objective == nullptr ? QString() : QString::fromLatin1(objective), QString::number(options.displacement_nodeset),
    options.displacement_component >= 0 && options.displacement_component < components.size() ? components[options.displacement_component] : QString(),
    options.adaptive_move_limits ? "adaptive_moves_on" : "adaptive_moves_off", QString::number(options.minimum_move_limit, 'g', 17)};
}

void TopologyManagement::refreshSettingsNodesets(int selected)
{
  settingsNodeset->clear();
  settingsNodeset->addItem("None", "0");
  CalculiXMeshInterface mesh;
  for (const auto& row : mesh.get_nodeset_tree_data())
  {
    if (row.size() >= 2)
    {
      settingsNodeset->addItem(QString("%1: %2").arg(QString::fromStdString(row[0]), QString::fromStdString(row[1])), QString::fromStdString(row[0]));
    }
  }
  if (selected > 0 && settingsNodeset->findData(QString::number(selected)) < 0)
  {
    settingsNodeset->addItem(QString("%1: missing nodeset").arg(selected), QString::number(selected));
  }
}

void TopologyManagement::setSettingsValues(const QStringList& values)
{
  if (values.size() != settingKeys.size())
  {
    return;
  }
  const QScopedValueRollback<bool> guard(loadingSettings, true);
  for (std::size_t i = 0; i < settingsFields.size(); ++i)
  {
    settingsFields[i]->setText(values[static_cast<int>(i)]);
  }
  refreshSettingsNodesets(values[11].toInt());
  const std::array<QComboBox*, 5> combos = {settingsMethod, settingsObjective, settingsNodeset, settingsComponent, settingsAdaptive};
  for (std::size_t i = 0; i < combos.size(); ++i)
  {
    const int index = combos[i]->findData(values[9 + static_cast<int>(i)]);
    combos[i]->setCurrentIndex(index >= 0 ? index : 0);
  }
  settingsMinimumMove->setText(values[14]);
  settingsComponent->setEnabled(settingsNodeset->currentData().toInt() > 0);
}

void TopologyManagement::rememberSettingsDraft()
{
  if (settingsDirty && (creating || currentTopologyId > 0))
  {
    settingsDrafts[creating ? 0 : currentTopologyId] = settingsValues();
  }
}

void TopologyManagement::loadSettings()
{
  settingsMessage->clear();
  settingsOriginal.clear();
  QStringList values;
  for (int i = 0; i < settingKeys.size(); ++i)
  {
    values.append("");
  }
  values[11] = "0";
  if (!creating && currentTopologyId > 0)
  {
    CalculiXTopologyInterface core;
    TopologyOptions options;
    if (!core.get_topology_settings(currentTopologyId, options))
    {
      return;
    }
    values = settingsValues(options);
    settingsOriginal = values;
    settingsId->setValue(currentTopologyId);
  }
  if (!creating && currentTopologyId <= 0)
  {
    settingsId->setValue(1);
  }
  settingsId->setEnabled(creating);
  const auto draft = settingsDrafts.find(creating ? 0 : currentTopologyId);
  settingsDirty = draft != settingsDrafts.end();
  if (settingsDirty)
  {
    values = draft->second;
    settingsMessage->setText("Unsaved edits retained. Apply to submit or Reload settings to discard.");
  }
  setSettingsValues(values);
  CalculiXTopologyInterface core;
  const bool running = !creating && currentTopologyId > 0 && core.get_topology_run_state(currentTopologyId) == "running";
  settingsForm->setEnabled((creating || currentTopologyId > 0) && !running && !animation_was_active);
  settingsApply->setEnabled(settingsForm->isEnabled());
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
  int nextId = 1;
  for (int i = 0; i < caseTree->topLevelItemCount(); ++i)
  {
    const int id = caseTree->topLevelItem(i)->data(0, Qt::UserRole).toInt();
    if (id >= nextId && id < std::numeric_limits<int>::max())
    {
      nextId = id + 1;
    }
  }
  settingsId->setValue(nextId);
  loadSettings();
  refresh();
  tabs->setCurrentIndex(SettingsTab);
}

void TopologyManagement::applySettings()
{
  if (submitting || animation_was_active || (!creating && currentTopologyId <= 0))
  {
    return;
  }
  CalculiXTopologyInterface core;
  if (!creating && core.get_topology_run_state(currentTopologyId) == "running")
  {
    PRINT_ERROR("Topology settings cannot be modified during a run.\n");
    return;
  }
  const int id = creating ? settingsId->value() : currentTopologyId;
  if (creating)
  {
    for (const auto& row : core.get_topology_tree_data())
    {
      if (!row.empty() && row[0] == std::to_string(id))
      {
        PRINT_ERROR("Topology %d already exists. Choose another ID.\n", id);
        return;
      }
    }
  }
  QStringList values = settingsValues();
  QString arguments;
  std::vector<int> supplied;
  for (int i = 0; i < values.size(); ++i)
  {
    if (!creating && settingsOriginal.size() == values.size() && values[i] == settingsOriginal[i])
    {
      continue;
    }
    QString value = values[i];
    if (value.isEmpty())
    {
      if (!creating)
      {
        PRINT_ERROR("A modified topology setting cannot be empty.\n");
        return;
      }
      continue;
    }
    if (i == 0)
    {
      if (value.contains('"') || value.contains('\n') || value.contains('\r'))
      {
        PRINT_ERROR("Topology names cannot contain quotes or line breaks.\n");
        return;
      }
      value = "\"" + value + "\"";
    }
    else if (i < 9 || i == 14)
    {
      bool valid = false;
      const double number = value.toDouble(&valid);
      if (!valid || !std::isfinite(number) || ((i == 3 || i == 8) && (number < 1 || number > std::numeric_limits<int>::max() || std::floor(number) != number)))
      {
        PRINT_ERROR("Invalid topology setting '%s'; use a finite number with a decimal point.\n", settingKeys[i].toUtf8().constData());
        return;
      }
      value = QString::number(number, 'g', 17);
      values[i] = value;
    }
    supplied.push_back(i);
    arguments += i == 13 ? " " + value : " " + settingKeys[i] + " " + value;
  }
  if (!creating && supplied.empty())
  {
    settingsMessage->setText("No settings changed.");
    return;
  }
  bool submitted = false;
  submitted = submitCommand(QString("ccx %1 topology %2").arg(creating ? "create" : "modify").arg(id) + arguments);
  // Submission is not command success. Compare the actual stored values before clearing edits.
  TopologyOptions stored;
  bool exists = false;
  for (const auto& row : core.get_topology_tree_data())
  {
    exists = exists || (!row.empty() && row[0] == std::to_string(id));
  }
  bool accepted = submitted && exists && core.get_topology_settings(id, stored);
  if (accepted)
  {
    const auto actual = settingsValues(stored);
    for (int i : supplied)
    {
      accepted = accepted && actual[i] == values[i];
    }
  }
  if (accepted)
  {
    settingsDrafts.erase(creating ? 0 : currentTopologyId);
    settingsDirty = false;
    creating = false;
    currentTopologyId = id;
    refresh();
    loadSettings();
    settingsMessage->setText("Stored topology settings loaded.");
  }
  else
  {
    settingsDirty = true;
    rememberSettingsDraft();
    settingsMessage->setText("Settings were not confirmed in the core. Entered values retained; see the command output.");
    refresh();
  }
}

void TopologyManagement::deleteTopology()
{
  if (submitting || creating || currentTopologyId <= 0 || animation_was_active)
  {
    return;
  }
  CalculiXTopologyInterface core;
  if (core.get_topology_run_state(currentTopologyId) == "running")
  {
    return;
  }
  submitCommand(QString("ccx delete topology %1").arg(currentTopologyId));
  refresh();
  loadSettings();
  refreshLists();
}
