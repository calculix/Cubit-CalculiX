#include "TopologySettingsPanel.hpp"
#include "CubitMessage.hpp"
#include "GuiSendCmd.hpp"

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
  const std::array<QString, 5> labels = {
    "Name", "Volume fraction", "Void stiffness ratio", "Maximum iterations", "Filter radius"
  };
  for (std::size_t i = 0; i < fields.size(); ++i)
  {
    fields[i] = new QLineEdit(settings);
    fields[i]->setPlaceholderText(modifying ? "" : "Optional: use default");
    form->addRow(labels[i], fields[i]);
  }
  fields[4]->setToolTip("0 selects the automatic filter radius; positive values use model length units.");

  method = new QComboBox(settings);
  method->addItem(modifying ? "Keep current" : "Default", "");
  method->addItem("Density", "density");
  method->addItem("Level set (not implemented)", "level_set");
  static_cast<QStandardItemModel *>(method->model())->item(2)->setEnabled(false);
  objective = new QComboBox(settings);
  objective->addItem(modifying ? "Keep current" : "Default", "");
  objective->addItem("Compliance", "compliance");
  objective->addItem("Mass (not implemented)", "mass");
  objective->addItem("Displacement (not implemented)", "displacement");
  for (int i = 2; i < objective->count(); ++i)
  {
    static_cast<QStandardItemModel *>(objective->model())->item(i)->setEnabled(false);
  }
  form->addRow("Method", method);
  form->addRow("Objective", objective);
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
  if (preferred_id <= 0)
  {
    preferred_id = cases->currentData().toInt();
  }
  {
    const QSignalBlocker blocker(cases);
    cases->clear();
    for (const auto &row : ccx_iface.get_topology_tree_data())
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

QStringList TopologySettingsPanel::values() const
{
  QStringList result;
  for (QLineEdit *field : fields)
  {
    result.append(field->text());
  }
  result.append(method->currentData().toString());
  result.append(objective->currentData().toString());
  return result;
}

void TopologySettingsPanel::load()
{
  loaded_id = -1;
  original.clear();
  settings->setEnabled(false);
  applyButton->setEnabled(false);
  for (QLineEdit *field : fields)
  {
    field->clear();
  }
  method->setCurrentIndex(0);
  objective->setCurrentIndex(0);
  if (cases->currentIndex() < 0)
  {
    return;
  }

  const int topology_id = cases->currentData().toInt();
  TopologyOptions options;
  if (!ccx_iface.get_topology_settings(topology_id, options))
  {
    return;
  }
  fields[0]->setText(QString::fromStdString(options.name));
  fields[1]->setText(QString::number(options.volume_fraction, 'g', 15));
  fields[2]->setText(QString::number(options.void_ratio, 'g', 15));
  fields[3]->setText(QString::number(options.maximum_iterations));
  fields[4]->setText(QString::number(options.filter_radius, 'g', 15));
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
  original = values();
  loaded_id = topology_id;
  const bool running = ccx_iface.get_topology_run_state(topology_id) == "running";
  settings->setEnabled(!running);
  applyButton->setEnabled(!running);
}

void TopologySettingsPanel::apply()
{
  const int topology_id = modifying ? cases->currentData().toInt() : id->value();
  if (modifying && (loaded_id != topology_id || original.size() != 7))
  {
    PRINT_ERROR("Load a topology case before modifying its settings.\n");
    return;
  }
  if (modifying && ccx_iface.get_topology_run_state(topology_id) == "running")
  {
    PRINT_ERROR("Topology %d is running; settings cannot be modified.\n", topology_id);
    return;
  }

  const QStringList current = values();
  const std::array<QString, 7> keywords = {
    "name", "volume_fraction", "void_ratio", "iterations", "filter_radius", "method", "objective"
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
      if (modifying && i < 5)
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
    else if (i < 5)
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
    arguments += " " + keywords[i] + " " + value;
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
