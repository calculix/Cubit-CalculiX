#include "Framework/Commands/UserOptionCommandSupport.hpp"
#include "PostprocessorOptionsPanel.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"

#include <QFileDialog>
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
  restartNotice->setText(active->user_options_restart_required()
    ? tr("Saved icons directory changes will take effect after restarting Cubit.")
    : tr("Icons directory changes require a restart."));
  baseline = option_values();
}

QJsonObject PostprocessorOptionsPanel::option_values() const
{
  QJsonObject values;
  values["paths/icons"] = iconsPath->text();
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
  if (!UserOptionCommandSupport::edited_commands("post useroptions", active->user_option_definitions(), edited, baseline, commands, error))
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
