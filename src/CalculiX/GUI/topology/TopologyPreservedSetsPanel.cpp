#include "TopologyPreservedSetsPanel.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include <string>
#include <QString>
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "CubitMessage.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QStringList>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <map>
#include <set>
#include <vector>

TopologyPreservedSetsPanel::TopologyPreservedSetsPanel(QWidget *parent)
  : QWidget(parent)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  QFormLayout *selection = new QFormLayout();

  cases = new QComboBox(this);
  QPushButton *reload = new QPushButton("Reload", this);

  QHBoxLayout *caseRow = new QHBoxLayout();
  caseRow->addWidget(cases);
  caseRow->addWidget(reload);
  selection->addRow("Topology case", caseRow);

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

  QObject::connect(cases, QOverload<int>::of(&QComboBox::currentIndexChanged),
    this, [this](int)
  {
    refreshLists();
  });
  QObject::connect(setType, QOverload<int>::of(&QComboBox::currentIndexChanged),
    this, [this](int)
  {
    refreshLists();
  });
  QObject::connect(reload, &QPushButton::clicked, this, [this]()
  {
    refreshCases();
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

void TopologyPreservedSetsPanel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshCases();
}

void TopologyPreservedSetsPanel::setTopology(int topology_id)
{
  refreshCases(topology_id);
}

void TopologyPreservedSetsPanel::refreshCases(int preferred_id)
{
  CalculiXTopologyInterface ccx_topology_iface;

  const bool explicitly_selected = preferred_id > 0;

  if (!explicitly_selected)
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

      if (!valid || topology_id <= 0)
      {
        continue;
      }

      cases->addItem(QString("%1: %2").arg(topology_id)
        .arg(QString::fromStdString(row[1])), topology_id);
    }

    const int index = cases->findData(preferred_id);

    if (index >= 0 || explicitly_selected)
    {
      cases->setCurrentIndex(index);
    }
  }

  refreshLists();
}

void TopologyPreservedSetsPanel::refreshLists()
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

  if (cases->currentIndex() < 0)
  {
    return;
  }

  const int topology_id = cases->currentData().toInt();
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
  editable = !state.empty() && state != "running";

  information->setText(
    QString("Automatically preserved through direct selections: %1 elements.")
      .arg(static_cast<qulonglong>(direct_elements.size())));

  updateButtons();
}

void TopologyPreservedSetsPanel::updateButtons()
{
  resetButton->setEnabled(editable);
  addButton->setEnabled(editable && !available->selectedItems().isEmpty());
  removeButton->setEnabled(editable && !preserved->selectedItems().isEmpty());
}

void TopologyPreservedSetsPanel::resetDefaults()
{
  const int topology_id = cases->currentData().toInt();

  if (!editable || topology_id <= 0 || loaded_id != topology_id)
  {
    return;
  }

  GuiSendCmd::gui_cmd(QString("ccx topology %1 reset preserved").arg(topology_id));

  // Reload actual state even if the command was rejected.
  refreshLists();
}

void TopologyPreservedSetsPanel::changeSelection(bool adding)
{
  CalculiXTopologyInterface ccx_topology_iface;

  const int topology_id = cases->currentData().toInt();
  const int type = setType->currentData().toInt();

  if (!editable || loaded_id != topology_id || loaded_type != type)
  {
    return;
  }

  const std::string state = ccx_topology_iface.get_topology_run_state(topology_id);

  if (state.empty() || state == "running")
  {
    PRINT_ERROR("Topology preservation cannot be changed at this time.\n");
    refreshCases();
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

  GuiSendCmd::gui_cmd(QString("ccx topology %1 %2 preserved %3 %4")
    .arg(topology_id).arg(adding ? "add" : "remove")
    .arg(keyword).arg(ids.join(" ")));

  // Reload actual core state even if the command was rejected.
  refreshLists();
}
