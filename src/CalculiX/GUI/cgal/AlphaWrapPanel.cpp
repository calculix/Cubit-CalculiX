#include "AlphaWrapPanel.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "CubitMessage.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QShowEvent>
#include <QSignalBlocker>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <cmath>
#include <set>
#include <utility>

AlphaWrapPanel::AlphaWrapPanel(QWidget* parent) : QWidget(parent)
{
  auto* layout = new QVBoxLayout(this);
  source = new QComboBox(this);
  source->addItem("Blocks", 0);
  source->addItem("Topology iteration", 1);
  auto* selection = new QFormLayout();
  selection->addRow("Source", source);
  layout->addLayout(selection);
  blocksGroup = new QGroupBox("Blocks", this);
  auto* blockLayout = new QVBoxLayout(blocksGroup);
  blocks = new QTreeWidget(blocksGroup);
  blocks->setRootIsDecorated(false);
  blocks->setHeaderLabels({"Name", "ID"});
  blocks->setSelectionMode(QAbstractItemView::ExtendedSelection);
  blockLayout->addWidget(blocks);
  layout->addWidget(blocksGroup);
  topologyGroup = new QGroupBox("Topology iteration", this);
  auto* topologyForm = new QFormLayout(topologyGroup);
  cases = new QComboBox(topologyGroup);
  iterations = new QComboBox(topologyGroup);
  threshold = new QDoubleSpinBox(topologyGroup);
  threshold->setRange(0.0, 1.0);
  threshold->setDecimals(6);
  threshold->setValue(0.5);
  topologyForm->addRow("Topology case", cases);
  topologyForm->addRow("Completed iteration", iterations);
  topologyForm->addRow("Threshold", threshold);
  layout->addWidget(topologyGroup);
  auto* form = new QFormLayout();
  alpha = new QLineEdit(this);
  offset = new QLineEdit(this);
  alpha->setPlaceholderText("Automatic");
  offset->setPlaceholderText("Automatic");
  alpha->setToolTip("Empty uses the input bounding-box diagonal / 40. Smaller alpha retains finer detail. Values use model length units.");
  offset->setToolTip("Empty uses the resolved alpha / 30. Smaller offset gives a tighter fit. Values use model length units.");
  filename = new QLineEdit(this);
  filename->setPlaceholderText("Optional: use a source-based STL filename");
  auto* browseButton = new QPushButton("Browse", this);
  auto* fileRow = new QHBoxLayout();
  fileRow->addWidget(filename, 1);
  fileRow->addWidget(browseButton);
  importStl = new QCheckBox("Import STL into Cubit", this);
  importStl->setChecked(true);
  overwrite = new QCheckBox("Overwrite existing STL", this);
  form->addRow("Alpha", alpha);
  form->addRow("Offset", offset);
  form->addRow("STL filename", fileRow);
  form->addRow(importStl);
  form->addRow(overwrite);
  layout->addLayout(form);
  information = new QLabel("Wrap output is an STL surface, not a new volume analysis mesh.", this);
  information->setWordWrap(true);
  information->setTextFormat(Qt::PlainText);
  layout->addWidget(information);
  auto* buttons = new QHBoxLayout();
  auto* reload = new QPushButton("Refresh", this);
  wrapButton = new QPushButton("Wrap", this);
  buttons->addWidget(reload);
  buttons->addStretch();
  buttons->addWidget(wrapButton);
  layout->addLayout(buttons);
  layout->addStretch();
  connect(source, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { updateControls(); });
  connect(cases, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { refreshIterations(); });
  connect(iterations, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { updateControls(); });
  connect(blocks, &QTreeWidget::itemSelectionChanged, this, [this]() { updateControls(); });
  connect(reload, &QPushButton::clicked, this, [this]() { refresh(); });
  connect(browseButton, &QPushButton::clicked, this, [this]() { browse(); });
  connect(wrapButton, &QPushButton::clicked, this, [this]() { wrap(); });
  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [this]()
  {
    if (isVisible() && !executing)
    {
      updateControls();
    }
  });
  timer->start(500);
  updateControls();
}

void AlphaWrapPanel::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  refresh();
}

void AlphaWrapPanel::refresh()
{
  if (executing)
  {
    return;
  }
  std::set<int> selected;
  for (auto* item : blocks->selectedItems())
  {
    selected.insert(item->data(0, Qt::UserRole).toInt());
  }
  {
    const QSignalBlocker blocker(blocks);
    blocks->clear();
    CalculiXMeshInterface mesh;
    for (const auto& row : mesh.get_blocks_tree_data())
    {
      if (row.size() < 2)
      {
        continue;
      }
      bool valid = false;
      const int id = QString::fromStdString(row[0]).toInt(&valid);
      if (valid && id > 0)
      {
        auto* item = new QTreeWidgetItem(blocks);
        item->setText(0, QString::fromStdString(row[1]));
        item->setText(1, QString::number(id));
        item->setData(0, Qt::UserRole, id);
        item->setSelected(selected.count(id) != 0);
      }
    }
  }
  const int preferred = cases->currentData().toInt();
  {
    const QSignalBlocker blocker(cases);
    cases->clear();
    CalculiXTopologyInterface topology;
    for (const auto& row : topology.get_topology_tree_data())
    {
      if (row.size() >= 2)
      {
        bool valid = false;
        const int id = QString::fromStdString(row[0]).toInt(&valid);
        if (valid && id > 0)
        {
          cases->addItem(QString("%1: %2").arg(id).arg(QString::fromStdString(row[1])), id);
        }
      }
    }
    const int index = cases->findData(preferred);
    cases->setCurrentIndex(index >= 0 ? index : (cases->count() > 0 ? 0 : -1));
  }
  refreshIterations();
}

void AlphaWrapPanel::refreshIterations()
{
  const int preferred = iterations->currentData().toInt();
  const QSignalBlocker blocker(iterations);
  iterations->clear();
  if (cases->currentIndex() >= 0)
  {
    CalculiXTopologyInterface topology;
    for (const auto& row : topology.get_topology_iteration_data(cases->currentData().toInt()))
    {
      if (row.size() >= 4 && row[2] == "2" && !row[3].empty())
      {
        iterations->addItem(QString("Iteration %1").arg(QString::fromStdString(row[0])), QString::fromStdString(row[0]).toInt());
      }
    }
    const int index = iterations->findData(preferred);
    iterations->setCurrentIndex(index >= 0 ? index : iterations->count() - 1);
  }
  updateControls();
}

void AlphaWrapPanel::updateControls()
{
  const bool topologySource = source->currentData().toInt() == 1;
  blocksGroup->setVisible(!topologySource);
  topologyGroup->setVisible(topologySource);
  CalculiXDrawInterface drawing;
  bool usable = !executing && !drawing.topology_animation_active();
  if (topologySource)
  {
    usable = usable && cases->currentIndex() >= 0 && iterations->currentIndex() >= 0;
    if (usable)
    {
      CalculiXTopologyInterface topology;
      const int id = cases->currentData().toInt();
      bool exists = false;
      for (const auto& row : topology.get_topology_tree_data())
      {
        exists = exists || (!row.empty() && row[0] == std::to_string(id));
      }
      if (exists)
      {
        const auto state = topology.get_topology_run_state(id);
        usable = !state.empty() && state != "running";
      }
      else
      {
        usable = false;
      }
    }
  }
  else
  {
    usable = usable && !blocks->selectedItems().isEmpty();
  }
  wrapButton->setEnabled(usable);
}

void AlphaWrapPanel::browse()
{
  const QString path = QFileDialog::getSaveFileName(this, "Export alpha wrap", filename->text(), "STL surface (*.stl)", nullptr, QFileDialog::DontConfirmOverwrite);
  if (!path.isEmpty())
  {
    filename->setText(QDir::fromNativeSeparators(path));
  }
}

void AlphaWrapPanel::wrap()
{
  updateControls();
  if (!wrapButton->isEnabled())
  {
    return;
  }
  QString command;
  QString defaultName;
  if (source->currentData().toInt() == 1)
  {
    const int id = cases->currentData().toInt();
    const int iteration = iterations->currentData().toInt();
    command = QString("ccx wrap topology %1 iteration %2 threshold %3").arg(id).arg(iteration).arg(QString::number(threshold->value(), 'f', 6));
    defaultName = QString("topo_%1_iteration_%2_wrap.stl").arg(id).arg(iteration);
  }
  else
  {
    QStringList ids;
    for (auto* item : blocks->selectedItems())
    {
      ids.append(QString::number(item->data(0, Qt::UserRole).toInt()));
    }
    command = "ccx wrap block " + ids.join(" ");
    defaultName = "blocks_" + ids.join("_") + "_wrap.stl";
  }
  for (const auto& parameter : {std::make_pair(alpha, QString("alpha")), std::make_pair(offset, QString("offset"))})
  {
    const QString text = parameter.first->text().trimmed();
    if (text.isEmpty())
    {
      continue;
    }
    bool valid = false;
    const double value = text.toDouble(&valid);
    if (!valid || !std::isfinite(value) || value <= 0.0)
    {
      PRINT_ERROR("%s must be a finite positive number or empty for automatic calculation.\n", parameter.second.toUtf8().constData());
      return;
    }
    command += " " + parameter.second + " " + QString::number(value, 'g', 17);
  }
  QString path = QDir::fromNativeSeparators(filename->text().trimmed());
  if (path.isEmpty())
  {
    path = defaultName;
  }
  if (path.contains('"') || path.contains('\n') || path.contains('\r'))
  {
    PRINT_ERROR("STL filenames cannot contain quotes or line breaks.\n");
    return;
  }
  command += " file \"" + path + "\"";
  if (!importStl->isChecked())
  {
    command += " no_import";
  }
  if (overwrite->isChecked())
  {
    command += " overwrite";
  }
  {
    const QScopedValueRollback<bool> guard(executing, true);
    wrapButton->setEnabled(false);
    GuiSendCmd::gui_cmd(command);
  }
  information->setText("Wrap command submitted. See command output for the result and resolved alpha/offset.");
  updateControls();
}

void AlphaWrapPanel::clearTopology(int topology_id)
{
  if (topology_id > 0 && cases->currentData().toInt() != topology_id)
  {
    return;
  }
  const QSignalBlocker caseBlocker(cases);
  const QSignalBlocker iterationBlocker(iterations);
  cases->clear();
  iterations->clear();
  if (topology_id <= 0)
  {
    const QSignalBlocker blockBlocker(blocks);
    blocks->clear();
    alpha->clear();
    offset->clear();
    filename->clear();
    threshold->setValue(0.5);
  }
  information->setText("Selection cleared. Refresh to load the current model.");
  wrapButton->setEnabled(false);
}
