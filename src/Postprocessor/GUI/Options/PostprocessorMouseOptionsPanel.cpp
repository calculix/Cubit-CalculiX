#include "PostprocessorMouseOptionsPanel.hpp"
#include "Postprocessor/Commands/options/PostprocessorOptionCommands.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <utility>
#include <type_traits>
#include <QVariant>

PostprocessorMouseOptionsPanel::PostprocessorMouseOptionsPanel(
  std::weak_ptr<PostprocessorApplicationInterface> active,
  std::function<bool(const std::string&, std::string&)> submit, QWidget* parent)
  : QWidget(parent), application(std::move(active)), submitCommand(std::move(submit))
{
  auto* layout = new QVBoxLayout(this);
  auto* grid = new QGridLayout;
  grid->addWidget(new QLabel(tr("Mouse Button"), this), 0, 0);
  grid->addWidget(new QLabel(tr("Function"), this), 0, 1);
  left = new QComboBox(this);
  middle = new QComboBox(this);
  right = new QComboBox(this);
  left->setObjectName("Postprocessor.Mouse.Left");
  middle->setObjectName("Postprocessor.Mouse.Middle");
  right->setObjectName("Postprocessor.Mouse.Right");
  for (auto* combo : {left, middle, right})
  {
    combo->addItem(tr("Rotate"), QStringLiteral("rotate"));
    combo->addItem(tr("Zoom"), QStringLiteral("zoom"));
    combo->addItem(tr("Pan"), QStringLiteral("pan"));
    combo->setToolTip(tr("Sets the drag action in Postprocessor viewports. Click actions are separate."));
  }
  grid->addWidget(new QLabel(tr("Left Button + Drag"), this), 1, 0);
  grid->addWidget(left, 1, 1);
  grid->addWidget(new QLabel(tr("Middle Button + Drag"), this), 2, 0);
  grid->addWidget(middle, 2, 1);
  grid->addWidget(new QLabel(tr("Right Button + Drag"), this), 3, 0);
  grid->addWidget(right, 3, 1);
  grid->addWidget(new QLabel(tr("Mouse Wheel"), this), 4, 0);
  grid->addWidget(new QLabel(tr("Zoom"), this), 4, 1);
  sensitivity = new QDoubleSpinBox(this);
  sensitivity->setRange(Postprocessor::DisplayDefaults::sensitivity_min, Postprocessor::DisplayDefaults::sensitivity_max);
  sensitivity->setDecimals(6);
  grid->addWidget(new QLabel(tr("Sensitivity"), this), 5, 0);
  grid->addWidget(sensitivity, 5, 1);
  reverseZoom = new QCheckBox(tr("Reverse Zoom"), this);
  grid->addWidget(reverseZoom, 6, 0, 1, 2);
  layout->addLayout(grid);
  auto* note = new QLabel(tr("These settings control Postprocessor viewports independently of Cubit. Saved changes apply to the next gesture."), this);
  note->setWordWrap(true);
  layout->addWidget(note);
  layout->addStretch();
  refresh_settings();
}

void PostprocessorMouseOptionsPanel::refresh_settings()
{
  const auto active = application.lock();
  setEnabled(active != nullptr);
  if (!active)
  {
    return;
  }
  const auto& options = active->user_options();
  left->setCurrentIndex(left->findData(QString::fromLatin1(Postprocessor::mouse_drag_action_name(options.leftDrag))));
  middle->setCurrentIndex(middle->findData(QString::fromLatin1(Postprocessor::mouse_drag_action_name(options.middleDrag))));
  right->setCurrentIndex(right->findData(QString::fromLatin1(Postprocessor::mouse_drag_action_name(options.rightDrag))));
  sensitivity->setValue(options.mouseSensitivity);
  reverseZoom->setChecked(options.reverseZoom);
  baseline = option_values();
}

QJsonObject PostprocessorMouseOptionsPanel::option_values() const
{
  QJsonObject values;
  values["mouse/left"] = left->currentData().toString();
  values["mouse/middle"] = middle->currentData().toString();
  values["mouse/right"] = right->currentData().toString();
  values["mouse/sensitivity"] = sensitivity->value();
  values["mouse/reversezoom"] = reverseZoom->isChecked();
  return values;
}

bool PostprocessorMouseOptionsPanel::save_settings(std::string& error)
{
  error.clear();
  const auto active = application.lock();
  if (!active || !submitCommand)
  {
    error = "Postprocessor mouse settings are unavailable.";
    return false;
  }
  const auto edited = option_values();
  std::vector<std::pair<std::string, std::string>> commands;
  if (!PostprocessorOptionCommands::edited_commands(active->user_option_definitions(), edited, baseline, commands, error))
  {
    return false;
  }
  for (const auto& command : commands)
  {
    if (!submitCommand(command.second, error))
    {
      return false;
    }
    UserOptionValues saved;
    if (!active->user_option_values(saved, error))
    {
      return false;
    }
    const auto key = QString::fromStdString(command.first);
    const auto found = saved.find(command.first);
    if (found == saved.end())
    {
      error = "The mouse command did not publish its setting.";
      return false;
    }
    const bool matches = std::visit([&](const auto& value)
    {
      using Type = std::decay_t<decltype(value)>;
      if constexpr (std::is_same_v<Type, std::string>)
      {
        return edited.value(key).toString().toStdString() == value;
      }
      else if constexpr (std::is_same_v<Type, bool>)
      {
        return edited.value(key).toBool() == value;
      }
      else
      {
        return edited.value(key).toDouble() == value;
      }
    }, found->second);
    if (!matches)
    {
      error = "The mouse command was rejected; see the Cubit error message.";
      return false;
    }
    baseline[key] = edited.value(key);
  }
  refresh_settings();
  return true;
}
