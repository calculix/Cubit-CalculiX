#include <type_traits>
#include "CalculiXDrawingToolbar.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "Core/gui/CalculiXGuiResourcesInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include <QAction>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSignalBlocker>
#include <QStyle>
#include <QSizePolicy>

CalculiXDrawingToolbar::CalculiXDrawingToolbar(QWidget* parent)
  : QToolBar(tr("CalculiX drawing"), parent)
{
  setObjectName("CalculiX.DrawingToolbar");
  setAllowedAreas(Qt::TopToolBarArea);
  setMovable(false);
  setFloatable(false);
  setToolButtonStyle(Qt::ToolButtonIconOnly);
  setIconSize(QSize(22, 22));
  CalculiXGuiResourcesInterface resources;
  overlayVisibility = addAction(resources.getIcon2("CCXDisplay"), tr("Display"));
  overlayVisibility->setObjectName("CalculiX.DrawingVisibility");
  overlayVisibility->setCheckable(true);
  overlayVisibility->setChecked(true);
  overlayVisibility->setToolTip(tr("Display\nShow or hide all CalculiX overlays while preserving selected groups and drawing settings"));
  connect(overlayVisibility, &QAction::triggered, this, [this](bool visible)
  {
    submit([&]() -> const char* {
      if (visible) {
        return "ccx display overlay on";
      } else {
        return "ccx display overlay off";
      }
    }());
  });
  addSeparator();
  symbolPicking = addAction(resources.getIcon2("CCXPickSymbols"), tr("Pick symbols"));
  symbolPicking->setObjectName("CalculiX.DrawingPicking");
  symbolPicking->setCheckable(true);
  symbolPicking->setToolTip(tr("Pick symbols\nSelect a CalculiX symbol in the viewport to show its model-tree item and properties. Turn off to restore Cubit's previous pick mode."));
  connect(symbolPicking, &QAction::triggered, this, [this](bool enabled)
  {
    submit([&]() -> const char* {
      if (enabled) {
        return "ccx display picking on";
      } else {
        return "ccx display picking off";
      }
    }());
  });
  addSeparator();
  labelVisibility = addAction(resources.getIcon2("CCXDrawingLabels"), tr("Labels"));
  labelVisibility->setObjectName("CalculiX.DrawingLabels");
  labelVisibility->setCheckable(true);
  labelVisibility->setToolTip(tr("Show or hide load and boundary-condition labels without changing symbols"));
  connect(labelVisibility, &QAction::triggered, this, [this](bool visible)
  {
    submit([&]() -> const char* {
      if (visible) {
        return "ccx display labels on";
      } else {
        return "ccx display labels off";
      }
    }());
  });
  addSeparator();
  scope = new QComboBox(this);
  scope->setObjectName("CalculiX.DrawingScope");
  scope->setMinimumWidth(90);
  scope->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  scope->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  scope->setToolTip(tr("Draw native reference conditions for the entire model or one step. Time-dependent amplitudes are not evaluated. Custom indicates an explicit selection made through commands."));
  addWidget(scope);
  addSeparator();
  drawAll = addAction(resources.getIcon2("CCXDrawAll"), tr("Draw all"));
  drawAll->setCheckable(true);
  drawAll->setToolTip(tr("Draw all\nIf all four groups are on, turn them all off. Otherwise turn them all on."));
  connect(drawAll, &QAction::triggered, this, [this]()
  {
    CalculiXDrawInterface drawing;
    submit([&]() -> const char* {
      if (allGroupsEnabled(drawing.drawing_state())) {
        return "ccx display all off";
      } else {
        return "ccx display all on";
      }
    }());
  });
  addSeparator();
  const auto addGroup = [&](DrawGroup group, const QString& label, const std::string& icon, const QString& keyword)
  {
    auto* action = addAction(resources.getIcon2(icon), label);
    action->setCheckable(true);
    action->setToolTip(tr("%1\nShow or hide this group in the selected scope").arg(label));
    groups.emplace(group, action);
    connect(action, &QAction::triggered, this, [this, keyword](bool checked)
    {
      submit("ccx display " + keyword + ([&]() -> const char* {
        if (checked) {
          return " on";
        } else {
          return " off";
        }
      }()));
    });
  };
  addGroup(DrawGroup::BoundaryConditions, tr("BCs"), "BCsTree", "bcs");
  addGroup(DrawGroup::Loads, tr("Loads"), "LoadsTree", "loads");
  addGroup(DrawGroup::Connections, tr("Connections"), "Connections", "connections");
  addGroup(DrawGroup::Constraints, tr("Constraints"), "ConstraintsTree", "constraints");
  addGroup(DrawGroup::Orientations, tr("Orientations"), "OrientationsTree", "orientations");
  addSeparator();
  auto* sizeIcon = new QLabel(this);
  sizeIcon->setPixmap(resources.getIcon2("CCXDrawingSize").pixmap(iconSize()));
  sizeIcon->setFixedSize(iconSize());
  sizeIcon->setAccessibleName(tr("Symbol size"));
  sizeIcon->setToolTip(tr("Symbol size factor relative to the model dimensions"));
  addWidget(sizeIcon);
  size = new QDoubleSpinBox(this);
  size->setObjectName("CalculiX.DrawingSize");
  size->setAccessibleName(tr("Symbol size"));
  size->setRange(0.01, 100.0);
  size->setDecimals(2);
  size->setSingleStep(0.1);
  size->setValue(1.0);
  size->setKeyboardTracking(false);
  size->setToolTip(tr("Symbol size factor relative to the model dimensions"));
  addWidget(size);
  auto* refresh = addAction(style()->standardIcon(QStyle::SP_BrowserReload), tr("Refresh"));
  refresh->setToolTip(tr("Refresh\nRebuild CalculiX symbols from the current model data"));
  connect(refresh, &QAction::triggered, this, [this]()
  {
    submit("ccx display refresh");
  });
  connect(scope, &QComboBox::activated, this, [this](int index)
  {
    const int id = scope->itemData(index).toInt();
    if (id == -2)
    {
      return;
    }
    submit([&]() -> QString {
      if (id < 0) {
        return QString("ccx display scope model");
      } else {
        return QString("ccx display scope step %1").arg(id);
      }
    }());
  });
  connect(size, &QDoubleSpinBox::valueChanged, this, [this](double value)
  {
    submit("ccx display size " + QString::number(value, 'g', 17));
  });
  refreshSteps();
  hide();
}

void CalculiXDrawingToolbar::submit(const QString& command)
{
  GuiSendCmd::gui_cmd(command);
  CalculiXDrawInterface drawing;
  synchronize(drawing.drawing_state());
}

void CalculiXDrawingToolbar::refreshSteps()
{
  const QSignalBlocker blocker(scope);
  const int selected = [&]() -> int {
    if (scope->currentIndex() < 0) {
      return -1;
    } else {
      return scope->currentData().toInt();
    }
  }();
  scope->clear();
  scope->addItem(tr("Entire model"), -1);
  CalculiXStepsInterface steps;
  for (const auto& row : steps.get_steps_tree_data())
  {
    if (row.size() < 2)
    {
      continue;
    }
    bool valid = false;
    const int id = QString::fromStdString(row[0]).toInt(&valid);
    if (valid && id >= 0)
    {
      scope->addItem(tr("Step %1 - %2").arg(id).arg(QString::fromStdString(row[1])), id);
    }
  }
  scope->addItem(tr("Custom (commands)"), -2);
  const int index = scope->findData(selected);
  scope->setCurrentIndex([&]() -> std::common_type_t<decltype(0), decltype(index)> {
    if (index < 0) {
      return 0;
    } else {
      return index;
    }
  }());
}

void CalculiXDrawingToolbar::synchronize(const CalculiXDrawState& state)
{
  CalculiXGuiResourcesInterface resources;
  const QSignalBlocker labelBlocker(labelVisibility);
  labelVisibility->setChecked(state.labels.enabled);
  const bool allOn = allGroupsEnabled(state);
  const QSignalBlocker drawAllBlocker(drawAll);
  drawAll->setChecked(allOn);
  drawAll->setIcon(resources.getIcon2([&]() -> const char* {
    if (allOn) {
      return "CCXDrawAllHidden";
    } else {
      return "CCXDrawAll";
    }
  }()));
  drawAll->setToolTip([&]() -> std::common_type_t<decltype(tr("Hide all\nTurn off BCs, Loads, Constraints and Orientations")), decltype(tr("Draw all\nTurn on BCs, Loads, Constraints and Orientations in the selected scope"))> {
    if (allOn) {
      return tr("Hide all\nTurn off BCs, Loads, Constraints and Orientations");
    } else {
      return tr("Draw all\nTurn on BCs, Loads, Constraints and Orientations in the selected scope");
    }
  }());
  const QSignalBlocker pickingBlocker(symbolPicking);
  symbolPicking->setChecked(state.pickingEnabled);
  symbolPicking->setIcon(resources.getIcon2([&]() -> const char* {
    if (state.pickingEnabled) {
      return "CCXPickSymbolsActive";
    } else {
      return "CCXPickSymbols";
    }
  }()));
  symbolPicking->setEnabled(state.overlayVisible);
  const QSignalBlocker overlayBlocker(overlayVisibility);
  overlayVisibility->setChecked(state.overlayVisible);
  overlayVisibility->setIcon(resources.getIcon2([&]() -> const char* {
    if (state.overlayVisible) {
      return "CCXDisplay";
    } else {
      return "CCXDisplayHidden";
    }
  }()));
  const QSignalBlocker scopeBlocker(scope);
  const QSignalBlocker sizeBlocker(size);
  const int id = [&]() -> int {
    if (state.scope.kind == DrawScopeKind::Custom) {
      return -2;
    } else {
      if (state.scope.kind == DrawScopeKind::Step) {
        return state.scope.stepId;
      } else {
        return -1;
      }
    }
  }();
  const int index = scope->findData(id);
  scope->setCurrentIndex([&]() -> std::common_type_t<decltype(0), decltype(index)> {
    if (index < 0) {
      return 0;
    } else {
      return index;
    }
  }());
  if (state.sizeFactor > size->maximum())
  {
    size->setMaximum(state.sizeFactor);
  }
  if (state.sizeFactor < size->minimum() && state.sizeFactor > 0)
  {
    size->setMinimum(state.sizeFactor);
  }
  size->setValue(state.sizeFactor);
  for (const auto& entry : groups)
  {
    const QSignalBlocker blocker(entry.second);
    const auto group = state.groups.find(entry.first);
    entry.second->setChecked(group != state.groups.end() && group->second.enabled);
  }
}

bool CalculiXDrawingToolbar::allGroupsEnabled(const CalculiXDrawState& state) const
{
  if (groups.empty())
  {
    return false;
  }
  for (const auto& entry : groups)
  {
    const auto group = state.groups.find(entry.first);
    if (group == state.groups.end() || !group->second.enabled)
    {
      return false;
    }
  }
  return true;
}
