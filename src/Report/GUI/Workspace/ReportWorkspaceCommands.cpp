#include "ReportWorkspace.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Report/Core/ReportSession.hpp"
#include "ReportCommandText.hpp"
#include "Report/GUI/Content/ReportContentTree.hpp"
#include "Report/GUI/Settings/ReportSettingsDialog.hpp"
#include "CubitMessage.hpp"
#include <QAction>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <QTimer>
#include <utility>

namespace
{
bool sameSource(const ReportRequest& a, const ReportRequest& b) {
  return a.provider == b.provider && a.kind == b.kind && a.model_id == b.model_id && a.job_id == b.job_id;
}

std::string sourceCommand(const ReportRequest& request, bool choose) {
  std::string command = "report ";
  if (choose) {
    command = "report source ";
  }
  command += request.provider;
  if (request.kind == "model") {
    command += " model";
  } else {
    command += " job " + std::to_string(request.job_id);
  }
  return command;
}
}

void ReportWorkspace::create_source_controls(QVBoxLayout* outer)
{
  auto* sourceRow = new QHBoxLayout();
  sources = new QComboBox(this);
  sources->setObjectName("ReportSourceSelector");
  sources->setPlaceholderText(tr("Choose a model or job"));
  sources->setMinimumContentsLength(25);
  sourceRow->addWidget(new QLabel(tr("Source"), this));
  sourceRow->addWidget(sources, 3);
  profiles = new QComboBox(this);
  profiles->setObjectName("ReportProfileSelector");
  sourceRow->addWidget(new QLabel(tr("Profile"), this));
  sourceRow->addWidget(profiles, 1);
  profileButton = new QPushButton(tr("Profiles"), this);
  profileButton->setObjectName("ReportProfileButton");
  auto* profileMenu = new QMenu(profileButton);
  for (const auto& entry : std::vector<std::pair<QString, std::string>>
    {{tr("Save as..."), "save"}, {tr("Load selected"), "load"}, {tr("Use selected as default"), "default"}}) {
    auto* action = profileMenu->addAction(entry.first);
    connect(action, &QAction::triggered, this, [this, operation = entry.second]() {
      profile_action(operation);
    });
  }
  profileButton->setMenu(profileMenu);
  sourceRow->addWidget(profileButton);
  outer->addLayout(sourceRow);
  connect(profiles, &QComboBox::activated, this, [this](int index) {
    if (index == 0) {
      dispatch("report profile reset");
    } else {
      profile_action("load");
    }
  });
  connect(sources, &QComboBox::activated, this, [this](int index) {
    choose_source(index);
  });
  auto* actions = new QHBoxLayout();
  generateButton = new QPushButton(tr("Generate"), this);
  generateButton->setObjectName("ReportGenerateButton");
  updateButton = new QPushButton(tr("Update"), this);
  settingsButton = new QPushButton(tr("Report settings"), this);
  settingsButton->setObjectName("ReportSettingsButton");
  auto* openPreview = new QPushButton(tr("Open preview / Export"), this);
  for (auto* button : {generateButton, updateButton, settingsButton, openPreview}) {
    actions->addWidget(button);
  }
  actions->addStretch();
  outer->addLayout(actions);
  connect(generateButton, &QPushButton::clicked, this, [this]() {
    generate();
  });
  connect(updateButton, &QPushButton::clicked, this, [this]() {
    dispatch("report update");
  });
  connect(openPreview, &QPushButton::clicked, this, [this]() {
    dispatch("report preview");
  });
  connect(settingsButton, &QPushButton::clicked, this, [this]() {
    const auto active = application.lock();
    if (!active) {
      return;
    }
    ReportTemplate definition;
    std::string error;
    if (!active->describe(definition, error)) {
      PRINT_ERROR("Cannot open Report settings: %s\n", error.c_str());
      return;
    }
    const auto& configuration = active->configuration_request().overrides;
    auto settings = configuration;
    if (settings.values.empty()) {
      settings = definition.defaults;
    }
    ReportSettingsDialog dialog(this, settings, application, submit);
    dialog.exec();
    refresh();
  });
  status = new QLabel(this);
  status->setWordWrap(true);
  outer->addWidget(status);
}

void ReportWorkspace::dispatch(const std::string& command, bool quiet) {
  std::string error;
  bool accepted = false;
  if (quiet) {
    accepted = silent(command, error);
  } else {
    accepted = submit(command, error);
  }
  if (!accepted) {
    PRINT_ERROR("Report command failed: %s\n", error.c_str());
  }
  QTimer::singleShot(0, this, [this]() {
    refresh();
  });
}

void ReportWorkspace::choose_source(int index) {
  if (index < 0 || !sources->itemData(index).isValid()) {
    return;
  }
  const int entry = sources->itemData(index).toInt();
  if (entry >= 0 && entry < static_cast<int>(entries.size()) && (entries[entry].available || entries[entry].has_report)) {
    const auto active = application.lock();
    if (active && sameSource(entries[entry].request, active->configuration_request())) {
      return;
    }
    {
      QSignalBlocker guard(profiles);
      profiles->setCurrentIndex(0);
    }
    profileWorkspace.clear();
    dispatch(sourceCommand(entries[entry].request, true));
  }
}

void ReportWorkspace::generate() {
  if (!sources->currentData().isValid()) {
    return;
  }
  const int index = sources->currentData().toInt();
  if (index < 0 || index >= static_cast<int>(entries.size()) || !entries[index].available) {
    return;
  }
  // Profile selection already loads its settings through a command. Generate
  // uses the current configuration, including edits made after loading it.
  dispatch(sourceCommand(entries[index].request, false));
}

void ReportWorkspace::profile_action(const std::string& action) {
  QString name;
  if (profiles->currentIndex() > 0) {
    name = profiles->currentText();
  }
  if (action == "save") {
    bool ok = false;
    name = QInputDialog::getText(this, tr("Save report profile"), tr("Profile name"), QLineEdit::Normal, name, &ok);
    if (!ok) {
      return;
    }
  }
  std::string quoted, error;
  if (!name.isEmpty() && ReportCommandText::quote(name, quoted, error)) {
    profileWorkspace.clear();
    dispatch("report profile " + action + " " + quoted);
  }
}

void ReportWorkspace::refresh_sources(ReportApplicationInterface& active, std::string& error)
{
  const auto& state = active.session();
  const auto& request = active.configuration_request();
  const bool sourcesAvailable = active.available_sources(entries, error);
  // Refresh only reflects Core state. Source commands require explicit activation.
  if (!sourcesAvailable) {
    entries.clear();
  }
  {
    QSignalBlocker guard(sources);
    sources->clear();
    std::string group;
    int selected = -1;
    for (std::size_t index = 0; index < entries.size(); ++index) {
      const auto& entry = entries[index];
      if (group != entry.workspace_label) {
        group = entry.workspace_label;
        sources->addItem(QString::fromStdString(group), -1);
        auto* item = qobject_cast<QStandardItemModel*>(sources->model())->item(sources->count() - 1);
        item->setFlags(Qt::NoItemFlags);
      }
      sources->addItem(QString::fromStdString(entry.label), static_cast<int>(index));
      auto* item = qobject_cast<QStandardItemModel*>(sources->model())->item(sources->count() - 1);
      item->setToolTip(QString::fromStdString(entry.unavailable_reason));
      if (!entry.available && !entry.has_report) {
        item->setEnabled(false);
      }
      if (sameSource(entry.request, request)) {
        selected = sources->count() - 1;
      }
    }
    sources->setCurrentIndex(selected);
  }
  int sourceIndex = -1;
  if (sources->currentData().isValid()) {
    sourceIndex = sources->currentData().toInt();
  }
  const bool sourceReady = sourcesAvailable && sourceIndex >= 0 && sourceIndex < static_cast<int>(entries.size()) &&
    (entries[sourceIndex].available || entries[sourceIndex].has_report) && sameSource(entries[sourceIndex].request, request);
  const bool canRegenerate = sourceReady && entries[sourceIndex].available;
  generateButton->setEnabled(canRegenerate);
  settingsButton->setEnabled(sourceReady);
  profileButton->setEnabled(sourceReady);
  profiles->setEnabled(sourceReady);
  tree->setEnabled(sourceReady);
  if (!sourceReady) {
    QSignalBlocker treeGuard(tree);
    QSignalBlocker profileGuard(profiles);
    tree->clear();
    profiles->clear();
    profileWorkspace.clear();
  }
  updateButton->setEnabled(canRegenerate && active.document() != nullptr && !state.pending_source);
  ReportTemplate definition;
  if (sourceReady && active.describe(definition, error)) {
    auto settings = request.overrides;
    if (settings.values.empty()) {
      settings = definition.defaults;
    }
    tree->refresh(definition, settings);
    if (profileWorkspace != definition.workspace_id) {
      std::vector<ReportProfile> available;
      if (active.profile("list", "", available, error)) {
        QSignalBlocker guard(profiles);
        profiles->clear();
        profiles->addItem(tr("Workspace default"));
        for (const auto& profile : available) {
          profiles->addItem(QString::fromStdString(profile.name), QString::fromStdString(profile.id));
        }
        profileWorkspace = definition.workspace_id;
      }
    }
    QSignalBlocker guard(profiles);
    const int selected = profiles->findData(QString::fromStdString(request.profile_id));
    if (selected >= 0) {
      profiles->setCurrentIndex(selected);
    } else {
      profiles->setCurrentIndex(0);
    }
  }
}
