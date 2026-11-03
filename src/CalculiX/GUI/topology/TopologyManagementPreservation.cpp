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

#include <set>
void TopologyManagement::createPreservationTab(QWidget* tab)
{
  auto* layout = new QVBoxLayout(tab);
  auto* selection = new QFormLayout();
  setType = new QComboBox(this);
  setType->addItem("Blocks", 1);
  setType->addItem("Nodesets", 2);
  setType->addItem("Sidesets", 3);
  selection->addRow("Set type", setType);

  layout->addLayout(selection);

  available = new QTreeWidget(this);
  preserved = new QTreeWidget(this);

  for (QTreeWidget *tree : {available, preserved})
  {
    tree->setColumnCount(3);
    tree->setHeaderLabels(QStringList() << "Name" << "ID" << "Status");
    tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tree->setRootIsDecorated(false);
  }

  QVBoxLayout *left = new QVBoxLayout();
  left->addWidget(new QLabel("Available / excluded", this));
  left->addWidget(available);

  QVBoxLayout *right = new QVBoxLayout();
  right->addWidget(new QLabel("Preserved", this));
  right->addWidget(preserved);

  addButton = new QPushButton("Add >", this);
  removeButton = new QPushButton("< Remove", this);

  QVBoxLayout *buttons = new QVBoxLayout();
  buttons->addStretch();
  buttons->addWidget(addButton);
  buttons->addWidget(removeButton);
  buttons->addStretch();

  QHBoxLayout *lists = new QHBoxLayout();
  lists->addLayout(left, 1);
  lists->addLayout(buttons);
  lists->addLayout(right, 1);
  layout->addLayout(lists, 1);

  information = new QLabel(this);
  information->setWordWrap(true);
  layout->addWidget(information);

  resetButton = new QPushButton("Restore automatic defaults", this);
  layout->addWidget(resetButton);

  QObject::connect(resetButton, &QPushButton::clicked, this, [this]()
  {
    resetDefaults();
  });

  QLabel *note = new QLabel(
    "Preservation changes apply to subsequent topology runs. "
    "An element can remain preserved through another set "
    "or a direct selection.", this);
  note->setWordWrap(true);
  layout->addWidget(note);

  QObject::connect(setType, QOverload<int>::of(&QComboBox::currentIndexChanged),
    this, [this](int)
  {
    refreshLists();
  });
  QObject::connect(addButton, &QPushButton::clicked, this, [this]()
  {
    changeSelection(true);
  });
  QObject::connect(removeButton, &QPushButton::clicked, this, [this]()
  {
    changeSelection(false);
  });
  QObject::connect(available, &QTreeWidget::itemSelectionChanged, this, [this]()
  {
    updateButtons();
  });
  QObject::connect(preserved, &QTreeWidget::itemSelectionChanged, this, [this]()
  {
    updateButtons();
  });
  QObject::connect(available, &QTreeWidget::itemDoubleClicked,
    this, [this](QTreeWidgetItem *, int)
  {
    changeSelection(true);
  });
  QObject::connect(preserved, &QTreeWidget::itemDoubleClicked,
    this, [this](QTreeWidgetItem *, int)
  {
    changeSelection(false);
  });

  updateButtons();
}

void TopologyManagement::refreshLists()
{
  CalculiXMeshInterface ccx_mesh_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  editable = false;
  loaded_id = -1;
  loaded_type = 0;

  available->clear();
  preserved->clear();
  information->clear();
  updateButtons();

  if (currentTopologyId <= 0 || creating)
  {
    return;
  }

  const int topology_id = currentTopologyId;
  const int type = setType->currentData().toInt();

  std::vector<TopologyPreservedSetInfo> rows;
  std::set<int> direct_elements;

  if (!ccx_topology_iface.get_topology_preserved_sets(topology_id, rows, direct_elements))
  {
    information->setText("Unable to read preservation settings. See the command line.");
    return;
  }

  std::vector<std::vector<std::string>> entities;

  if (type == 1)
  {
    entities = ccx_mesh_iface.get_blocks_tree_data();
  }
  else if (type == 2)
  {
    entities = ccx_mesh_iface.get_nodeset_tree_data();
  }
  else if (type == 3)
  {
    entities = ccx_mesh_iface.get_sideset_tree_data();
  }
  else
  {
    return;
  }

  std::map<int, QString> names;
  std::set<int> existing;

  for (const auto& entity : entities)
  {
    if (entity.size() < 2)
    {
      continue;
    }

    bool valid = false;
    const int set_id = QString::fromStdString(entity[0]).toInt(&valid);

    if (valid && set_id > 0)
    {
      names[set_id] = QString::fromStdString(entity[1]);
      existing.insert(set_id);
    }
  }

  std::map<int, TopologyPreservedSetInfo> details;

  for (const auto& row : rows)
  {
    if (row.set_type != type)
    {
      continue;
    }

    details[row.set_id] = row;

    if (names.count(row.set_id) == 0)
    {
      names[row.set_id] = QString("Missing set %1").arg(row.set_id);
    }
  }

  for (const auto& named : names)
  {
    TopologyPreservedSetInfo state;
    const auto detail = details.find(named.first);

    if (detail != details.end())
    {
      state = detail->second;
    }

    QStringList status;

    if (state.automatic)
    {
      status.append("Automatic");
    }
    if (state.user_added)
    {
      status.append("Explicitly included");
    }
    if (state.excluded)
    {
      status.append("Excluded");
    }
    if (status.isEmpty())
    {
      status.append("Available");
    }

    QTreeWidget *target = state.effective ? preserved : available;
    QTreeWidgetItem *item = new QTreeWidgetItem(target);
    item->setText(0, named.second);
    item->setText(1, QString::number(named.first));
    item->setText(2, status.join(", "));
    item->setData(0, Qt::UserRole, named.first);

    if (existing.count(named.first) == 0)
    {
      item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
      item->setToolTip(0, "This stored reference no longer exists in the model.");
    }
  }

  const std::string state = ccx_topology_iface.get_topology_run_state(topology_id);
  loaded_id = topology_id;
  loaded_type = type;
  editable = !state.empty() && state != "running" && !animation_was_active;

  information->setText(
    QString("Automatically preserved through direct selections: %1 elements.")
      .arg(static_cast<qulonglong>(direct_elements.size())));

  updateButtons();
}

void TopologyManagement::updateButtons()
{
  resetButton->setEnabled(editable && !submitting);
  addButton->setEnabled(editable && !available->selectedItems().isEmpty());
  removeButton->setEnabled(editable && !preserved->selectedItems().isEmpty());
}

void TopologyManagement::resetDefaults()
{
  const int topology_id = currentTopologyId;

  if (!editable || topology_id <= 0 || loaded_id != topology_id)
  {
    return;
  }

  submitCommand(QString("ccx topology %1 reset preserved").arg(topology_id));

  // Reload actual state even if the command was rejected.
  refreshLists();
}

void TopologyManagement::changeSelection(bool adding)
{
  CalculiXTopologyInterface ccx_topology_iface;

  const int topology_id = currentTopologyId;
  const int type = setType->currentData().toInt();

  if (!editable || loaded_id != topology_id || loaded_type != type)
  {
    return;
  }

  const std::string state = ccx_topology_iface.get_topology_run_state(topology_id);

  if (state.empty() || state == "running")
  {
    PRINT_ERROR("Topology preservation cannot be changed at this time.\n");
    refreshLists();
    return;
  }

  QTreeWidget *source = adding ? available : preserved;
  QStringList ids;

  for (QTreeWidgetItem *item : source->selectedItems())
  {
    if (!(item->flags() & Qt::ItemIsEnabled))
    {
      continue;
    }

    const int set_id = item->data(0, Qt::UserRole).toInt();

    if (set_id > 0)
    {
      ids.append(QString::number(set_id));
    }
  }

  if (ids.isEmpty())
  {
    return;
  }

  QString keyword;

  if (type == 1)
  {
    keyword = "block";
  }
  else if (type == 2)
  {
    keyword = "nodeset";
  }
  else if (type == 3)
  {
    keyword = "sideset";
  }
  else
  {
    return;
  }

  submitCommand(QString("ccx topology %1 %2 preserved %3 %4")
    .arg(topology_id).arg(adding ? "add" : "remove")
    .arg(keyword).arg(ids.join(" ")));

  // Reload actual core state even if the command was rejected.
  refreshLists();
}
