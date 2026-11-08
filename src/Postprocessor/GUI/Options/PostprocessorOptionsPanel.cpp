#include "Postprocessor/Commands/options/PostprocessorOptionCommands.hpp"
#include "PostprocessorOptionsPanel.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"

#include <QFileDialog>
#include <QColorDialog>
#include <QGroupBox>
#include <QVariant>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <utility>

PostprocessorOptionsPanel::PostprocessorOptionsPanel(std::weak_ptr<PostprocessorApplicationInterface> application,
  std::function<bool(const std::string&, std::string&)> submit, QWidget* parent)
  : QWidget(parent), application(std::move(application)), submitCommand(std::move(submit))
{
  auto* layout = new QVBoxLayout(this);
  auto* form = new QFormLayout;
  auto* pathRow = new QHBoxLayout;
  iconsPath = new QLineEdit(this);
  iconsPath->setToolTip(tr("Relative to the component directory, or an absolute path. Changes require a restart."));
  auto* browse = new QPushButton(tr("Browse…"), this);
  pathRow->addWidget(iconsPath);
  pathRow->addWidget(browse);
  form->addRow(tr("Icons directory"), pathRow);
  layout->addLayout(form);
  restartNotice = new QLabel(this);
  restartNotice->setWordWrap(true);
  layout->addWidget(restartNotice);
  auto* displayGroup = new QGroupBox(tr("Display"), this);
  auto* displayForm = new QFormLayout(displayGroup);
  backgroundTop = new QPushButton(displayGroup);
  backgroundBottom = new QPushButton(displayGroup);
  displayForm->addRow(tr("Screen Top"), backgroundTop);
  displayForm->addRow(tr("Screen Bottom"), backgroundBottom);
  triad = new QCheckBox(tr("Display Triad"), displayGroup);
  displayForm->addRow(triad);
  layout->addWidget(displayGroup);
  for (auto* button : {backgroundTop, backgroundBottom})
  {
    connect(button, &QPushButton::clicked, this, [this, button]()
    {
      choose_color(button);
    });
  }
  journalCamera = new QCheckBox(tr("Record camera navigation"), this);
  journalCamera->setToolTip(tr("Record completed GUI rotate, pan, zoom and fit actions. Typed commands follow Cubit's journal settings. Takes effect immediately."));
  layout->addWidget(new QLabel(tr("Journal"), this));
  layout->addWidget(journalCamera);
  auto* journalNote = new QLabel(tr("Source and result operations always use the normal journaled command path."), this);
  journalNote->setWordWrap(true);
  layout->addWidget(journalNote);
  layout->addStretch();
  connect(browse, &QPushButton::clicked, this, [this]()
  {
    const auto path = QFileDialog::getExistingDirectory(this, tr("Select Postprocessor icons directory"), iconsPath->text());
    if (!path.isEmpty())
    {
      iconsPath->setText(path);
    }
  });
  refresh_settings();
}

void PostprocessorOptionsPanel::refresh_settings()
{
  const auto active = application.lock();
  setEnabled(active != nullptr);
  if (!active)
  {
    return;
  }
  iconsPath->setText(QString::fromUtf8(active->user_options().iconsPath.c_str()));
  const auto& options = active->user_options();
  journalCamera->setChecked(options.journalCamera);
  triad->setChecked(options.triadVisible);
  const std::pair<QPushButton*, std::array<double, 3>> colors[] =
  {
    {backgroundTop, options.backgroundTop}, {backgroundBottom, options.backgroundBottom}
  };
  for (const auto& entry : colors)
  {
    const auto color = QColor::fromRgbF(entry.second[0], entry.second[1], entry.second[2]);
    entry.first->setProperty("rgb", QString::fromStdString(Postprocessor::format_background_color(entry.second)));
    entry.first->setText(color.name());
    entry.first->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(color.name(), color.lightnessF() > 0.5 ? "black" : "white"));
  }
  restartNotice->setText(active->user_options_restart_required()
    ? tr("Saved icons directory changes will take effect after restarting Cubit.")
    : tr("Icons directory changes require a restart."));
  baseline = option_values();
}

QJsonObject PostprocessorOptionsPanel::option_values() const
{
  QJsonObject values;
  values["paths/icons"] = iconsPath->text();
  values["journal/camera"] = journalCamera->isChecked();
  values["background/top"] = backgroundTop->property("rgb").toString();
  values["background/bottom"] = backgroundBottom->property("rgb").toString();
  values["display/triad"] = triad->isChecked();
  return values;
}

bool PostprocessorOptionsPanel::save_settings(std::string& error)
{
  error.clear();
  const auto active = application.lock();
  if (!active || !submitCommand)
  {
    error = "Postprocessor user options are unavailable.";
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
    const auto key = QString::fromStdString(command.first);
    baseline[key] = edited.value(key);
  }
  refresh_settings();
  return true;
}

void PostprocessorOptionsPanel::choose_color(QPushButton* button)
{
  const auto color = QColorDialog::getColor(QColor(button->text()), this, tr("Background Color"));
  if (!color.isValid())
  {
    return;
  }
  button->setProperty("rgb", QString::fromStdString(Postprocessor::format_background_color({color.redF(), color.greenF(), color.blueF()})));
  button->setText(color.name());
  button->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(color.name(), color.lightnessF() > 0.5 ? "black" : "white"));
}
