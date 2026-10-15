#include "TopologyDeletePanel.hpp"
#include "CubitMessage.hpp"
#include "GuiSendCmd.hpp"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QVBoxLayout>

TopologyDeletePanel::TopologyDeletePanel(QWidget *parent)
  : QWidget(parent)
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

  QLabel *description = new QLabel(
    "Deletes the selected topology case and its linked "
    "job/result records. The current mesh is retained.", this);
  description->setWordWrap(true);
  layout->addWidget(description);

  deleteButton = new QPushButton("Delete case and linked jobs", this);
  layout->addWidget(deleteButton);
  layout->addStretch();

  QObject::connect(cases, QOverload<int>::of(&QComboBox::currentIndexChanged),
    this, [this](int)
  {
    updateButton();
  });
  QObject::connect(reload, &QPushButton::clicked, this, [this]()
  {
    refreshCases();
  });
  QObject::connect(deleteButton, &QPushButton::clicked, this, [this]()
  {
    deleteCase();
  });

  updateButton();
}

void TopologyDeletePanel::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  refreshCases();
}

void TopologyDeletePanel::refreshCases()
{
  const int preferred_id = cases->currentData().toInt();

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

    if (index >= 0)
    {
      cases->setCurrentIndex(index);
    }
  }

  updateButton();
}

void TopologyDeletePanel::updateButton()
{
  deleteButton->setEnabled(false);

  if (cases->currentIndex() < 0)
  {
    return;
  }

  const std::string state = ccx_iface.get_topology_run_state(cases->currentData().toInt());
  deleteButton->setEnabled(!state.empty() && state != "running");
}

void TopologyDeletePanel::deleteCase()
{
  if (cases->currentIndex() < 0)
  {
    return;
  }

  const int topology_id = cases->currentData().toInt();
  const std::string state = ccx_iface.get_topology_run_state(topology_id);

  if (state.empty() || state == "running")
  {
    PRINT_ERROR("The topology case cannot be deleted at this time.\n");
    updateButton();
    return;
  }

  GuiSendCmd::gui_cmd(QString("ccx delete topology %1").arg(topology_id));
  refreshCases();
}
