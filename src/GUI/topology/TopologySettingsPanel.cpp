#include "TopologySettingsPanel.hpp"
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
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <cmath>
#include <limits>

TopologySettingsPanel::TopologySettingsPanel(bool modify, QWidget *parent)
  : QWidget(parent), modifying(modify)
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
  const std::array<QString, 8> labels = {
    "Name", "Volume fraction", "Void stiffness ratio", "Maximum iterations",
    "Filter radius", "Density move limit", "Density tolerance", "Relative objective tolerance"
  };
  for (std::size_t i = 0; i < fields.size(); ++i)
  {
    fields[i] = new QLineEdit(settings);
    fields[i]->setPlaceholderText(modifying ? "" : "Optional: use default");
    form->addRow(labels[i], fields[i]);
  }
  fields[4]->setToolTip("0 selects the automatic filter radius; positive values use model length units.");
  fields[5]->setToolTip("Maximum density change per iteration; greater than 0 and at most 1.");
  fields[6]->setToolTip("Maximum density change allowed for convergence; greater than 0 and at most 1.");
  fields[7]->setToolTip("Relative objective change allowed for convergence; 0.001 corresponds to 0.1 percent.");

  adaptiveMoves = new QComboBox(settings);
  adaptiveMoves->addItem(modifying ? "Keep current" : "Default", "");
  adaptiveMoves->addItem("Off", "adaptive_moves_off");
  adaptiveMoves->addItem("On", "adaptive_moves_on");
  adaptiveMoves->setToolTip("Adapt the compliance density move limit from previous density changes. Other objectives do not use this setting.");

  minimumMoveLimit = new QLineEdit(settings);
  minimumMoveLimit->setPlaceholderText(modifying ? "" : "Optional: use default");
  minimumMoveLimit->setToolTip("Minimum adaptive move limit; 0 automatically uses 10 percent of the maximum density move limit. An explicit value must not exceed that maximum.");

  form->insertRow(6, "Adaptive compliance moves", adaptiveMoves);
  form->insertRow(7, "Minimum move limit", minimumMoveLimit);

  method = new QComboBox(settings);
  method->addItem(modifying ? "Keep current" : "Default", "");
  method->addItem("Density", "density");
  method->addItem("Level set (not implemented)", "level_set");
  static_cast<QStandardItemModel *>(method->model())->item(2)->setEnabled(false);
  objective = new QComboBox(settings);
  objective->addItem(modifying ? "Keep current" : "Default", "");
  objective->addItem("Compliance", "compliance");
  objective->addItem("Mass (not implemented)", "mass");
  objective->addItem("Displacement", "displacement");
  static_cast<QStandardItemModel *>(objective->model())->item(2)->setEnabled(false);
  form->addRow("Method", method);
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
  layout->addWidget(settings);
  layout->addStretch();
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
    refreshNodesets();
    refreshCases();
  });
  QObject::connect(applyButton, &QPushButton::clicked, this, [this]()
  {
    apply();
  });
}

bool TopologySettingsPanel::isModify() const
{
  return modifying;
}

void TopologySettingsPanel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshNodesets();
  if (modifying)
  {
    refreshCases();
  }
}

void TopologySettingsPanel::setTopology(int topology_id)
{
  if (modifying)
  {
    refreshCases(topology_id);
  }
}

void TopologySettingsPanel::refreshCases(int preferred_id)
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

void TopologySettingsPanel::refreshNodesets(int preferred_id)
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

QStringList TopologySettingsPanel::values() const
{
  QStringList result;
  for (QLineEdit *field : fields)
  {
    result.append(field->text());
  }
  result.append(method->currentData().toString());
  result.append(objective->currentData().toString());
  const int nodeset_id = displacementNodeset->currentData().toInt();
  const bool include_measurement = modifying || nodeset_id > 0;
  result.append(include_measurement ? QString::number(nodeset_id) : QString());
  result.append(include_measurement ? displacementComponent->currentData().toString() : QString());
  result.append(adaptiveMoves->currentData().toString());
  result.append(minimumMoveLimit->text());
  return result;
}

void TopologySettingsPanel::load()
{
  CalculiXTopologyInterface ccx_topology_iface;

  loaded_id = -1;
  original.clear();
  settings->setEnabled(false);
  applyButton->setEnabled(false);
  for (QLineEdit *field : fields)
  {
    field->clear();
  }
  adaptiveMoves->setCurrentIndex(0);
  minimumMoveLimit->clear();
  method->setCurrentIndex(0);
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
  fields[0]->setText(QString::fromStdString(options.name));
  fields[1]->setText(QString::number(options.volume_fraction, 'g', 15));
  fields[2]->setText(QString::number(options.void_ratio, 'g', 15));
  fields[3]->setText(QString::number(options.maximum_iterations));
  fields[4]->setText(QString::number(options.filter_radius, 'g', 15));
  fields[5]->setText(QString::number(options.move_limit, 'g', 15));
  fields[6]->setText(QString::number(options.density_tolerance, 'g', 15));
  fields[7]->setText(QString::number(options.compliance_tolerance, 'g', 15));
  adaptiveMoves->setCurrentIndex(adaptiveMoves->findData(options.adaptive_move_limits ? "adaptive_moves_on" : "adaptive_moves_off"));
  minimumMoveLimit->setText(QString::number(options.minimum_move_limit, 'g', 15));
  if (options.method == TopologyMethod::Density)
  {
    method->setCurrentIndex(1);
  }
  else if (options.method == TopologyMethod::LevelSet)
  {
    method->setCurrentIndex(2);
  }
  if (options.objective == TopologyObjective::Compliance)
  {
    objective->setCurrentIndex(1);
  }
  else if (options.objective == TopologyObjective::Mass)
  {
    objective->setCurrentIndex(2);
  }
  else if (options.objective == TopologyObjective::Displacement)
  {
    objective->setCurrentIndex(3);
  }
  refreshNodesets(options.displacement_nodeset);
  displacementComponent->setCurrentIndex(options.displacement_component);
  original = values();
  loaded_id = topology_id;
  const bool running = ccx_topology_iface.get_topology_run_state(topology_id) == "running";
  settings->setEnabled(!running);
  applyButton->setEnabled(!running);
}

void TopologySettingsPanel::apply()
{
  CalculiXTopologyInterface ccx_topology_iface;

  const int topology_id = modifying ? cases->currentData().toInt() : id->value();
  if (modifying && (loaded_id != topology_id || original.size() != static_cast<qsizetype>(fields.size() + 6)))
  {
    PRINT_ERROR("Load a topology case before modifying its settings.\n");
    return;
  }
  if (modifying && ccx_topology_iface.get_topology_run_state(topology_id) == "running")
  {
    PRINT_ERROR("Topology %d is running; settings cannot be modified.\n", topology_id);
    return;
  }

  const QStringList current = values();
  if (objective->currentData().toString() == "displacement" && displacementNodeset->currentData().toInt() <= 0)
  {
    PRINT_ERROR("Select a nodeset for the displacement objective.\n");
    return;
  }
  const std::array<QString, 14> keywords = {
    "name", "volume_fraction", "void_ratio", "iterations", "filter_radius",
    "move_limit", "density_tolerance", "compliance_tolerance", "method", "objective",
    "displacement_nodeset", "displacement_component", "adaptive_moves", "minimum_move_limit"
  };
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
    else if (i == 3)
    {
      bool valid = false;
      const int iterations = value.toInt(&valid);
      if (!valid || iterations <= 0)
      {
        PRINT_ERROR("Topology maximum iterations must be a positive integer.\n");
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
    if (keywords[i] == "adaptive_moves")
    {
      arguments += " " + value;
    }
    else
    {
      arguments += " " + keywords[i] + " " + value;
    }
  }
  if (modifying && arguments.isEmpty())
  {
    PRINT_INFO("No topology settings were changed.\n");
    return;
  }

  // gui_cmd does not report command success. Keep the entered values available
  // so a rejected command does not discard the user's edits.
  GuiSendCmd::gui_cmd(QString("ccx %1 topology %2")
    .arg(modifying ? "modify" : "create").arg(topology_id) + arguments);
}
