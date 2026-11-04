#include "TopologyDensitySettingsPanel.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include <array>
#include <QString>
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "CubitMessage.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <cmath>
#include <algorithm>
#include <limits>

TopologyDensitySettingsPanel::TopologyDensitySettingsPanel(bool modify, QWidget *parent)
  : QWidget(parent), modifying(modify), storedMethod(TopologyMethod::Unspecified)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  QFormLayout *selection = new QFormLayout();
  layout->addLayout(selection);

  id = new QSpinBox(this);
  id->setRange(1, std::numeric_limits<int>::max());
  cases = new QComboBox(this);
  QPushButton *reload = new QPushButton("Reload", this);
  if (modifying)
  {
    id->hide();
    QHBoxLayout *caseRow = new QHBoxLayout();
    caseRow->addWidget(cases);
    caseRow->addWidget(reload);
    selection->addRow("Topology case", caseRow);
  }
  else
  {
    cases->hide();
    reload->hide();
    selection->addRow("Topology ID", id);
  }

  settings = new QWidget(this);
  QFormLayout *form = new QFormLayout(settings);
  const QStringList labels = {"Name", "Volume fraction", "Void stiffness ratio", "Maximum iterations", "Material cap", "Relative objective tolerance", "Filter radius", "Density move limit", "Density tolerance", "Minimum move limit"};
  for (std::size_t i = 0; i < fields.size(); ++i)
  {
    fields[i] = new QLineEdit(settings);
    fields[i]->setPlaceholderText(modifying ? "" : "Optional: use Core default");
    form->addRow(labels[static_cast<int>(i)], fields[i]);
  }
  fields[5]->setToolTip("Relative objective change allowed for convergence; 0.001 corresponds to 0.1 percent.");
  fields[4]->setToolTip("Maximum generated topology materials across all base materials. Minimum: 2.");
  fields[6]->setToolTip("Filter radius in model length units. Zero selects automatic radius.");
  fields[7]->setToolTip("Maximum density change per update.");
  fields[8]->setToolTip("Maximum density change and ordinary proposal residual allowed for convergence.");
  fields[9]->setToolTip("Minimum adaptive move limit. Zero selects ten percent of the maximum density move limit.");
  adaptiveMoves = new QComboBox(settings);
  adaptiveMoves->addItem(modifying ? "Keep current" : "Default", "");
  adaptiveMoves->addItem("Off", "adaptive_moves_off");
  adaptiveMoves->addItem("On", "adaptive_moves_on");
  adaptiveMoves->setToolTip("Adapt density moves for compliance and displacement objectives.");
  form->addRow("Adaptive density moves", adaptiveMoves);
  objective = new QComboBox(settings);
  objective->addItem(modifying ? "Keep current" : "Default", "");
  objective->addItem("Compliance", "compliance");
  objective->addItem("Mass (not implemented)", "mass");
  objective->addItem("Displacement", "displacement");
  static_cast<QStandardItemModel *>(objective->model())->item(2)->setEnabled(false);
  form->addRow("Objective", objective);
  displacementNodeset = new QComboBox(settings);
  displacementNodeset->setToolTip("Required for the displacement objective. For compliance, selecting a nodeset enables displacement monitoring; None disables monitoring.");
  displacementComponent = new QComboBox(settings);
  displacementComponent->addItem("Magnitude", "magnitude");
  displacementComponent->addItem("X", "x");
  displacementComponent->addItem("Y", "y");
  displacementComponent->addItem("Z", "z");
  displacementComponent->setToolTip("Global coordinates. X/Y/Z measure the maximum absolute component; magnitude measures the maximum vector norm.");
  form->addRow("Displacement nodeset", displacementNodeset);
  form->addRow("Displacement component", displacementComponent);
  QObject::connect(displacementNodeset, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int)
  {
    displacementComponent->setEnabled(displacementNodeset->currentData().toInt() > 0);
  });
  refreshNodesets(0);
  auto* scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setWidget(settings);
  layout->addWidget(scroll, 1);
  feedback = new QLabel(this);
  feedback->setWordWrap(true);
  feedback->setTextFormat(Qt::PlainText);
  layout->addWidget(feedback);
  applyButton = new QPushButton("Apply", this);
  layout->addWidget(applyButton, 0, Qt::AlignRight);

  if (modifying)
  {
    settings->setEnabled(false);
    applyButton->setEnabled(false);
  }
  QObject::connect(cases, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int)
  {
    load();
  });
  QObject::connect(reload, &QPushButton::clicked, this, [this]()
  {
    drafts.erase(loaded_id);
    loaded_id = -1;
    refreshNodesets();
    refreshCases();
  });
  QObject::connect(applyButton, &QPushButton::clicked, this, [this]()
  {
    apply();
  });
}

bool TopologyDensitySettingsPanel::isModify() const
{
  return modifying;
}

void TopologyDensitySettingsPanel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshNodesets();
  if (modifying)
  {
    refreshCases();
  }
}

void TopologyDensitySettingsPanel::setTopology(int topology_id)
{
  if (modifying)
  {
    refreshCases(topology_id);
  }
}

void TopologyDensitySettingsPanel::refreshCases(int preferred_id)
{
  CalculiXTopologyInterface ccx_topology_iface;

  if (preferred_id <= 0)
  {
    preferred_id = cases->currentData().toInt();
  }
  {
    const QSignalBlocker blocker(cases);
    cases->clear();
    for (const auto &row : ccx_topology_iface.get_topology_tree_data())
    {
      if (row.size() < 2)
      {
        continue;
      }
      bool valid = false;
      const int topology_id = QString::fromStdString(row[0]).toInt(&valid);
      if (valid && topology_id > 0)
      {
        cases->addItem(QString("%1: %2").arg(topology_id).arg(QString::fromStdString(row[1])), topology_id);
      }
    }
    const int index = cases->findData(preferred_id);
    if (index >= 0)
    {
      cases->setCurrentIndex(index);
    }
  }
  load();
}

void TopologyDensitySettingsPanel::refreshNodesets(int preferred_id)
{
  CalculiXMeshInterface ccx_mesh_iface;

  if (preferred_id < 0)
  {
    preferred_id = displacementNodeset->currentData().toInt();
  }

  const QSignalBlocker blocker(displacementNodeset);
  displacementNodeset->clear();
  displacementNodeset->addItem("None", 0);

  for (const auto& row : ccx_mesh_iface.get_nodeset_tree_data())
  {
    if (row.size() < 2)
    {
      continue;
    }

    bool valid = false;
    const int nodeset_id = QString::fromStdString(row[0]).toInt(&valid);

    if (valid && nodeset_id > 0)
    {
      displacementNodeset->addItem(QString("%1: %2").arg(nodeset_id).arg(QString::fromStdString(row[1])), nodeset_id);
    }
  }

  int index = displacementNodeset->findData(preferred_id);

  if (index < 0 && preferred_id > 0)
  {
    displacementNodeset->addItem(QString("%1: missing nodeset").arg(preferred_id), preferred_id);
    index = displacementNodeset->count() - 1;
  }

  displacementNodeset->setCurrentIndex(index >= 0 ? index : 0);
  displacementComponent->setEnabled(displacementNodeset->currentData().toInt() > 0);
}

QStringList TopologyDensitySettingsPanel::values() const
{
  QStringList result;
  for (QLineEdit *field : fields)
  {
    result.append(field->text().trimmed());
  }
  result.append(objective->currentData().toString());
  const int nodeset_id = displacementNodeset->currentData().toInt();
  const bool include_measurement = modifying || nodeset_id > 0;
  result.append(include_measurement ? QString::number(nodeset_id) : QString());
  result.append(include_measurement ? displacementComponent->currentData().toString() : QString());
  result.append(adaptiveMoves->currentData().toString());
  return result;
}

void TopologyDensitySettingsPanel::load()
{
  CalculiXTopologyInterface ccx_topology_iface;

  rememberDraft();
  loaded_id = -1;
  original.clear();
  settings->setEnabled(false);
  applyButton->setEnabled(false);
  for (QLineEdit *field : fields)
  {
    field->clear();
  }
  adaptiveMoves->setCurrentIndex(0);
  objective->setCurrentIndex(0);
  displacementNodeset->setCurrentIndex(0);
  displacementComponent->setCurrentIndex(0);
  displacementComponent->setEnabled(false);
  if (cases->currentIndex() < 0)
  {
    return;
  }

  const int topology_id = cases->currentData().toInt();
  TopologyOptions options;
  if (!ccx_topology_iface.get_topology_settings(topology_id, options))
  {
    return;
  }
  storedMethod = options.method;
  fields[0]->setText(QString::fromStdString(options.name));
  fields[1]->setText(QString::number(options.volume_fraction, 'g', 17));
  fields[2]->setText(QString::number(options.void_ratio, 'g', 17));
  fields[3]->setText(QString::number(options.maximum_iterations));
  fields[4]->setText(QString::number(options.material_cap));
  fields[5]->setText(QString::number(options.compliance_tolerance, 'g', 17));
  fields[6]->setText(QString::number(options.filter_radius, 'g', 17));
  fields[7]->setText(QString::number(options.move_limit, 'g', 17));
  fields[8]->setText(QString::number(options.density_tolerance, 'g', 17));
  fields[9]->setText(QString::number(options.minimum_move_limit, 'g', 17));
  const char* objectiveName = topology_objective_name(options.objective);
  const int objectiveIndex = objective->findData(objectiveName == nullptr ? QString() : QString::fromLatin1(objectiveName));
  objective->setCurrentIndex(objectiveIndex >= 0 ? objectiveIndex : 0);
  adaptiveMoves->setCurrentIndex(adaptiveMoves->findData(options.adaptive_move_limits ? "adaptive_moves_on" : "adaptive_moves_off"));
  refreshNodesets(options.displacement_nodeset);
  displacementComponent->setCurrentIndex(options.displacement_component);
  original = values();
  const auto draft = drafts.find(topology_id);
  if (draft != drafts.end() && draft->second.size() == original.size())
  {
    for (std::size_t i = 0; i < fields.size(); ++i)
    {
      fields[i]->setText(draft->second[static_cast<int>(i)]);
    }
    objective->setCurrentIndex(std::max(0, objective->findData(draft->second[10])));
    refreshNodesets(draft->second[11].toInt());
    displacementComponent->setCurrentIndex(std::max(0, displacementComponent->findData(draft->second[12])));
    adaptiveMoves->setCurrentIndex(std::max(0, adaptiveMoves->findData(draft->second[13])));
  }

  loaded_id = topology_id;
  const bool running = ccx_topology_iface.get_topology_run_state(topology_id) == "running";
  settings->setEnabled(!running);
  applyButton->setEnabled(!running);
}

void TopologyDensitySettingsPanel::apply()
{
  CalculiXTopologyInterface ccx_topology_iface;

  const int topology_id = modifying ? cases->currentData().toInt() : id->value();
  if (modifying && (loaded_id != topology_id || original.size() != 14))
  {
    PRINT_ERROR("Load a topology case before modifying its settings.\n");
    return;
  }
  if (modifying && ccx_topology_iface.get_topology_run_state(topology_id) == "running")
  {
    PRINT_ERROR("Topology %d is running; settings cannot be modified.\n", topology_id);
    return;
  }

  if (!modifying)
  {
    for (const auto& row : ccx_topology_iface.get_topology_tree_data())
    {
      if (!row.empty() && row[0] == std::to_string(topology_id))
      {
        PRINT_ERROR("Topology %d already exists. Choose another ID.\n", topology_id);
        return;
      }
    }
  }
  const QStringList current = values();
  QStringList requested = current;
  std::vector<int> supplied;
  if (objective->currentData().toString() == "displacement" && displacementNodeset->currentData().toInt() <= 0)
  {
    PRINT_ERROR("Select a nodeset for the displacement objective.\n");
    return;
  }
  const QStringList keywords = {"name", "volume_fraction", "void_ratio", "iterations", "material_cap", "compliance_tolerance", "filter_radius", "move_limit", "density_tolerance", "minimum_move_limit", "objective", "displacement_nodeset", "displacement_component", "adaptive_moves"};
  QString arguments;
  for (int i = 0; i < current.size(); ++i)
  {
    if (modifying && current[i] == original[i])
    {
      continue;
    }
    QString value = current[i].trimmed();
    if (value.isEmpty())
    {
      if (modifying && (i < static_cast<int>(fields.size()) || keywords[i] == "minimum_move_limit"))
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
        PRINT_ERROR("Topology names cannot contain double quotes or line breaks.\n");
        return;
      }
      value = "\"" + value + "\"";
    }
    else if (keywords[i] == "iterations" || keywords[i] == "material_cap" || keywords[i] == "level_set_integration_max_refinements")
    {
      bool valid = false;
      const double count = value.toDouble(&valid);
      const bool refinements = keywords[i] == "level_set_integration_max_refinements";
      const double maximum = refinements ? 5.0 : static_cast<double>((std::numeric_limits<int>::max)());
      const int iterations = valid && std::isfinite(count) && count >= 1.0 && count <= maximum && std::floor(count) == count ? static_cast<int>(count) : 0;
      if (!valid || iterations <= 0)
      {
        PRINT_ERROR("Topology count settings must be positive integers.\n");
        return;
      }
      value = QString::number(iterations);
    }
    else if (i < static_cast<int>(fields.size()) || keywords[i] == "minimum_move_limit")
    {
      bool valid = false;
      const double number = value.toDouble(&valid);
      if (!valid || !std::isfinite(number))
      {
        PRINT_ERROR("Invalid topology numeric setting '%s'; use a decimal point.\n",
          keywords[i].toStdString().c_str());
        return;
      }
      value = QString::number(number, 'g', 17);
    }
    if (i != 0)
    {
      requested[i] = value;
    }
    supplied.push_back(i);
    if (keywords[i] == "adaptive_moves")
    {
      arguments += " " + value;
    }
    else
    {
      arguments += " " + keywords[i] + " " + value;
    }
  }
  if (!modifying || storedMethod != TopologyMethod::Density)
  {
    arguments += " method density";
  }
  if (modifying && arguments.isEmpty())
  {
    PRINT_INFO("No topology settings were changed.\n");
    return;
  }

  // Submission alone is not success; confirm every supplied value in Core.
  const QString command = QString("ccx %1 topology %2").arg(modifying ? "modify" : "create").arg(topology_id) + arguments;
  const bool submitted = GuiSendCmd::gui_cmd(command);
  TopologyOptions stored;
  bool exists = false;
  for (const auto& row : ccx_topology_iface.get_topology_tree_data())
  {
    exists = exists || (!row.empty() && row[0] == std::to_string(topology_id));
  }
  bool accepted = submitted && exists && ccx_topology_iface.get_topology_settings(topology_id, stored) && stored.method == TopologyMethod::Density;
  if (accepted)
  {
    const auto actual = values(stored);
    for (const int i : supplied)
    {
      accepted = accepted && actual[i] == requested[i];
    }
  }
  if (!accepted)
  {
    rememberDraft();
    feedback->setText("Settings were not confirmed in Core. Entered values retained; see command output.");
    return;
  }
  drafts.erase(topology_id);
  loaded_id = -1;
  feedback->setText("Stored topology settings confirmed.");
  if (modifying)
  {
    load();
  }
}

void TopologyDensitySettingsPanel::clearTopology(int topology_id)
{
  feedback->clear();
  if (topology_id <= 0)
  {
    drafts.clear();
  }
  else
  {
    drafts.erase(topology_id);
  }
  if (topology_id > 0 && (!modifying || cases->currentData().toInt() != topology_id))
  {
    return;
  }
  if (modifying)
  {
    const QSignalBlocker blocker(cases);
    cases->clear();
    loaded_id = -1;
    load();
  }
  else
  {
    for (auto* field : fields)
    {
      field->clear();
    }
    objective->setCurrentIndex(0);
    adaptiveMoves->setCurrentIndex(0);
    const QSignalBlocker blocker(displacementNodeset);
    displacementNodeset->clear();
    displacementComponent->setCurrentIndex(0);
    displacementComponent->setEnabled(false);
    id->setValue(1);
  }
}

void TopologyDensitySettingsPanel::rememberDraft()
{
  if (loaded_id > 0 && !original.empty())
  {
    if (values() != original)
    {
      drafts[loaded_id] = values();
    }
    else
    {
      drafts.erase(loaded_id);
    }
  }
}

QStringList TopologyDensitySettingsPanel::values(const TopologyOptions& options) const
{
  QStringList result;
  result.append(QString::fromStdString(options.name));
  result.append(QString::number(options.volume_fraction, 'g', 17));
  result.append(QString::number(options.void_ratio, 'g', 17));
  result.append(QString::number(options.maximum_iterations));
  result.append(QString::number(options.material_cap));
  result.append(QString::number(options.compliance_tolerance, 'g', 17));
  result.append(QString::number(options.filter_radius, 'g', 17));
  result.append(QString::number(options.move_limit, 'g', 17));
  result.append(QString::number(options.density_tolerance, 'g', 17));
  result.append(QString::number(options.minimum_move_limit, 'g', 17));
  const char* objectiveName = topology_objective_name(options.objective);
  const QStringList components = {"magnitude", "x", "y", "z"};
  result.append(objectiveName == nullptr ? QString() : QString::fromLatin1(objectiveName));
  result.append(QString::number(options.displacement_nodeset));
  result.append(options.displacement_component >= 0 && options.displacement_component < components.size() ? components[options.displacement_component] : QString());
  result.append(options.adaptive_move_limits ? "adaptive_moves_on" : "adaptive_moves_off");
  return result;
}
