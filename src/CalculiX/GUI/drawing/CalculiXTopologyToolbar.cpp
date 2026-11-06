#include "CalculiXTopologyToolbar.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "CubitMessage.hpp"
#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSettings>
#include <QSignalBlocker>
#include <QScopedValueRollback>
#include <QStyle>
#include <QSizePolicy>
#include <QTimer>

CalculiXTopologyToolbar::CalculiXTopologyToolbar(QWidget* parent)
  : QToolBar(tr("CalculiX topology"), parent)
{
  setObjectName("CalculiX.TopologyToolbar");
  setAllowedAreas(Qt::TopToolBarArea);
  setMovable(false);
  setFloatable(false);
  setToolButtonStyle(Qt::ToolButtonIconOnly);
  setIconSize(QSize(22, 22));
  addSeparator();
  cases = new QComboBox(this);
  cases->setMinimumWidth(90);
  cases->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  cases->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  cases->setToolTip(tr("Select the topology case to preview"));
  addWidget(cases);
  const auto navigation = [this](QStyle::StandardPixmap icon, const QString& label, const QString& tip, int destination)
  {
    auto* action = addAction(style()->standardIcon(icon), label);
    action->setToolTip(tip);
    connect(action, &QAction::triggered, this, [this, destination]()
    {
      const int index = destination == -2 ? 0 : destination == 2 ? iterations->count() - 1 : iterations->currentIndex() + destination;
      if (index >= 0 && index < iterations->count())
      {
        iterations->setCurrentIndex(index);
      }
    });
    return action;
  };
  first = navigation(QStyle::SP_MediaSkipBackward, tr("First"), tr("Show the first completed iteration"), -2);
  previous = navigation(QStyle::SP_MediaSeekBackward, tr("Previous"), tr("Show the previous completed iteration"), -1);
  iterations = new QComboBox(this);
  iterations->setMinimumWidth(80);
  iterations->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  iterations->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  iterations->setToolTip(tr("Select a completed result iteration; updates the drawing when Topology Preview is enabled"));
  addWidget(iterations);
  next = navigation(QStyle::SP_MediaSeekForward, tr("Next"), tr("Show the next completed iteration"), 1);
  last = navigation(QStyle::SP_MediaSkipForward, tr("Last"), tr("Show the last completed iteration"), 2);
  CalculiXGuiResourcesInterface resources;
  auto* thresholdIcon = new QLabel(this);
  thresholdIcon->setPixmap(resources.getIcon2("CCXTopologyThreshold").pixmap(iconSize()));
  thresholdIcon->setFixedSize(iconSize());
  thresholdIcon->setAccessibleName(tr("Density threshold"));
  thresholdIcon->setToolTip(tr("Density threshold: elements below this density are shown as red edges; preserved elements always remain green"));
  addWidget(thresholdIcon);
  threshold = new QDoubleSpinBox(this);
  threshold->setAccessibleName(tr("Density threshold"));
  threshold->setDecimals(6);
  threshold->setRange(0, 1);
  threshold->setSingleStep(0.05);
  threshold->setKeyboardTracking(false);
  QSettings settings("cubit-sim", "Framework");
  threshold->setValue(settings.value("TopologyToolbar/Threshold", 0.5).toDouble());
  threshold->setToolTip(tr("Density threshold: elements below this density are shown as red edges; preserved elements always remain green"));
  addWidget(threshold);
  visibility = addAction(resources.getIcon2("Topology"), tr("Topology Preview"));
  visibility->setCheckable(true);
  visibility->setToolTip(tr("Show or hide topology preview; showing a preview also enables CalculiX display"));
  wrap = addAction(resources.getIcon2("CCXCGALWrap"), tr("CGAL Alpha wrap"));
  wrap->setToolTip(tr("Wrap the selected iteration using the displayed density threshold, automatic alpha and offset, overwrite the STL, and import it into Cubit."));
  connect(wrap, &QAction::triggered, this, [this]() { wrapIteration(); });
  previewTimer = new QTimer(this);
  previewTimer->setSingleShot(true);
  connect(previewTimer, &QTimer::timeout, this, [this]() { preview(); });
  connect(cases, &QComboBox::activated, this, [this]() { refreshIterations(); selectionChanged(); });
  connect(iterations, &QComboBox::currentIndexChanged, this, [this]() { selectionChanged(); });
  connect(threshold, &QDoubleSpinBox::valueChanged, this, [this](double value)
  {
    QSettings("cubit-sim", "Framework").setValue("TopologyToolbar/Threshold", value);
    selectionChanged();
  });
  connect(visibility, &QAction::triggered, this, [this](bool enabled)
  {
    previewTimer->stop();
    if (enabled)
    {
      preview();
    }
    else
    {
      GuiSendCmd::gui_cmd("ccx display topology off");
      synchronize();
    }
  });
  auto* timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [this]() { refresh(); });
  timer->start(500);
  refresh();
  hide();
}

void CalculiXTopologyToolbar::cancelPendingPreview()
{
  previewTimer->stop();
}

void CalculiXTopologyToolbar::invalidateTopology(int id)
{
  if (cases->currentIndex() >= 0 && cases->currentData().toInt() == id)
  {
    previewTimer->stop();
    const QSignalBlocker blocker(iterations);
    iterations->clear();
    synchronize();
  }
}

void CalculiXTopologyToolbar::clear()
{
  previewTimer->stop();
  const QSignalBlocker caseBlocker(cases);
  const QSignalBlocker iterationBlocker(iterations);
  cases->clear();
  iterations->clear();
  synchronize();
}

void CalculiXTopologyToolbar::refresh()
{
  if (busy || previewTimer->isActive())
  {
    return;
  }
  CalculiXTopologyInterface topology;
  const int selected = cases->currentData().toInt();
  const int iteration = iterations->currentData().toInt();
  const QSignalBlocker blocker(cases);
  cases->clear();
  for (const auto& row : topology.get_topology_tree_data())
  {
    if (row.size() >= 2)
    {
      bool valid = false;
      const int id = QString::fromStdString(row[0]).toInt(&valid);
      if (valid && id > 0)
      {
        cases->addItem(QString("%1 - %2").arg(id).arg(QString::fromStdString(row[1])), id);
      }
    }
  }
  const int index = cases->findData(selected);
  cases->setCurrentIndex(index < 0 ? (cases->count() > 0 ? 0 : -1) : index);
  refreshIterations(index < 0 ? -1 : iteration);
  CalculiXDrawInterface drawing;
  const auto state = drawing.drawing_state();
  const auto group = state.groups.find(DrawGroup::Topology);
  if (state.overlayVisible && group != state.groups.end() && group->second.enabled)
  {
    int caseId = -1;
    int iterationId = -1;
    double densityThreshold = 0.5;
    drawing.topology_preview(caseId, iterationId, densityThreshold);
    const int previewCase = cases->findData(caseId);
    if (previewCase >= 0)
    {
      cases->setCurrentIndex(previewCase);
      refreshIterations(iterationId);
      const QSignalBlocker thresholdBlocker(threshold);
      threshold->setValue(densityThreshold);
    }
  }
  synchronize();
}

void CalculiXTopologyToolbar::refreshIterations(int selected)
{
  const QSignalBlocker blocker(iterations);
  iterations->clear();
  if (cases->currentIndex() >= 0)
  {
    CalculiXTopologyInterface topology;
    // Iteration row: ID, physical job ID, physical job status, objective, ...
    for (const auto& row : topology.get_topology_iteration_data(cases->currentData().toInt()))
    {
      if (row.size() > 3 && row[2] == "2" && !row[3].empty())
      {
        bool valid = false;
        const int id = QString::fromStdString(row[0]).toInt(&valid);
        if (valid && id > 0)
        {
          iterations->addItem(tr("Iteration %1").arg(id), id);
        }
      }
    }
  }
  const int index = iterations->findData(selected);
  iterations->setCurrentIndex(index < 0 ? iterations->count() - 1 : index);
}

void CalculiXTopologyToolbar::synchronize()
{
  CalculiXDrawInterface drawing;
  const auto state = drawing.drawing_state();
  const auto group = state.groups.find(DrawGroup::Topology);
  const bool shown = state.overlayVisible && group != state.groups.end() && group->second.enabled;
  const QSignalBlocker blocker(visibility);
  visibility->setChecked(shown);
  const bool available = !busy && cases->currentIndex() >= 0 && iterations->currentIndex() >= 0 && !drawing.topology_animation_active() && CalculiXTopologyInterface().get_topology_run_state(cases->currentData().toInt()) != "running";
  wrap->setEnabled(available);
  visibility->setEnabled(shown || available);
  threshold->setEnabled(available);
  const int index = iterations->currentIndex();
  first->setEnabled(available && index > 0);
  previous->setEnabled(available && index > 0);
  next->setEnabled(available && index + 1 < iterations->count());
  last->setEnabled(available && index + 1 < iterations->count());
}

void CalculiXTopologyToolbar::selectionChanged()
{
  if (visibility->isChecked())
  {
    previewTimer->start(75);
  }
  else
  {
    synchronize();
  }
}

void CalculiXTopologyToolbar::preview()
{
  if (busy || cases->currentIndex() < 0 || iterations->currentIndex() < 0 || CalculiXDrawInterface().topology_animation_active() || CalculiXTopologyInterface().get_topology_run_state(cases->currentData().toInt()) == "running")
  {
    synchronize();
    return;
  }
  {
    const QScopedValueRollback<bool> guard(busy, true);
    const auto command = QString("ccx result project topology %1 iteration %2 threshold %3 preview").arg(cases->currentData().toInt()).arg(iterations->currentData().toInt()).arg(QString::number(threshold->value(), 'f', 6));
    CalculiXApplicationInterface().silent_cmd(command.toStdString());
  }
  synchronize();
}

void CalculiXTopologyToolbar::wrapIteration()
{
  if (busy)
  {
    return;
  }

  previewTimer->stop();
  synchronize();

  if (!wrap->isEnabled())
  {
    return;
  }

  const int topologyId = cases->currentData().toInt();
  const int iterationId = iterations->currentData().toInt();
  const QString filename = QString("topo_%1_iteration_%2_wrap.stl").arg(topologyId).arg(iterationId);
  const QString command = QString("ccx wrap topology %1 iteration %2 threshold %3 file \"%4\" overwrite").arg(topologyId).arg(iterationId).arg(QString::number(threshold->value(), 'f', 6)).arg(filename);

  {
    const QScopedValueRollback<bool> guard(busy, true);
    synchronize();

    if (!GuiSendCmd::gui_cmd(command))
    {
      PRINT_ERROR("Unable to submit the topology Alpha wrap command.\n");
    }
  }

  refresh();
}
