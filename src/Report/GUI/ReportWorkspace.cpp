#include "ReportWorkspace.hpp"
#include "ReportToolbar.hpp"
#include "ReportExportDialog.hpp"
#include "ReportContentTree.hpp"
#include "ReportSettingsDialog.hpp"
#include "ReportPreviewWidget.hpp"
#include "ReportPreviewWindow.hpp"
#include "ReportCommandText.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "CubitMessage.hpp"
#include <QMainWindow>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QComboBox>
#include <QInputDialog>
#include <QMenu>
#include <QTimer>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <utility>

namespace
{
bool sameSource(const ReportRequest& a, const ReportRequest& b)
{
  return a.provider == b.provider && a.kind == b.kind && a.model_id == b.model_id && a.job_id == b.job_id;
}

std::string sourceCommand(const ReportRequest& request, bool choose)
{
  return std::string(choose ? "report source " : "report ") + request.provider +
    (request.kind == "model" ? " model" : " job " + std::to_string(request.job_id));
}
}

ReportWorkspace::ReportWorkspace(QWidget* host, std::weak_ptr<ReportApplicationInterface> active,
  std::function<bool(const std::string&, std::string&)> send,
  std::function<bool(const std::string&, std::string&)> quiet, const QIcon& image,
  const QIcon& pdfIcon, const QIcon& odtIcon)
  : QWidget(host), application(std::move(active)), submit(std::move(send)), silent(std::move(quiet)), icon(image)
{
  setObjectName("ReportWorkspace");
  toolbar = new ReportToolbar(this, pdfIcon, odtIcon, [this](const std::string& format)
  {
    std::string error;
    if (!ReportExportDialog::run(this, format, submit, error))
    {
      PRINT_ERROR("Cannot export report: %s\n", error.c_str());
    }
  });
  auto* outer = new QVBoxLayout(this);
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
    {{tr("Save as..."), "save"}, {tr("Load selected"), "load"}, {tr("Use selected as default"), "default"}})
  {
    auto* action = profileMenu->addAction(entry.first);
    connect(action, &QAction::triggered, this, [this, operation = entry.second]()
    {
      profile_action(operation);
    });
  }
  profileButton->setMenu(profileMenu);
  sourceRow->addWidget(profileButton);
  outer->addLayout(sourceRow);
  connect(profiles, &QComboBox::activated, this, [this](int index)
  {
    if (index == 0)
    {
      dispatch("report profile reset");
    }
    else
    {
      profile_action("load");
    }
  });
  connect(sources, &QComboBox::activated, this, [this](int index)
  {
    choose_source(index);
  });
  auto* actions = new QHBoxLayout();
  generateButton = new QPushButton(tr("Generate"), this);
  generateButton->setObjectName("ReportGenerateButton");
  updateButton = new QPushButton(tr("Update"), this);
  settingsButton = new QPushButton(tr("Report settings"), this);
  settingsButton->setObjectName("ReportSettingsButton");
  auto* openPreview = new QPushButton(tr("Open preview / Export"), this);
  for (auto* button : {generateButton, updateButton, settingsButton, openPreview})
  {
    actions->addWidget(button);
  }
  actions->addStretch();
  outer->addLayout(actions);
  connect(generateButton, &QPushButton::clicked, this, [this]()
  {
    generate();
  });
  connect(updateButton, &QPushButton::clicked, this, [this]()
  {
    dispatch("report update");
  });
  connect(openPreview, &QPushButton::clicked, this, [this]()
  {
    dispatch("report preview");
  });
  connect(settingsButton, &QPushButton::clicked, this, [this]()
  {
    const auto active = application.lock();
    if (!active)
    {
      return;
    }
    ReportTemplate definition;
    std::string error;
    if (!active->describe(definition, error))
    {
      PRINT_ERROR("Cannot open Report settings: %s\n", error.c_str());
      return;
    }
    const auto& configuration = active->configuration_request().overrides;
    ReportSettingsDialog dialog(this, configuration.values.empty() ? definition.defaults : configuration, application, submit);
    dialog.exec();
    refresh();
  });
  status = new QLabel(this);
  status->setWordWrap(true);
  outer->addWidget(status);
  navigationBar = new QWidget(this);
  auto* navigation = new QHBoxLayout(navigationBar);
  navigation->setContentsMargins(0, 0, 0, 0);
  auto* previous = new QPushButton(tr("Previous"), this);
  auto* next = new QPushButton(tr("Next"), this);
  pageNumber = new QSpinBox(this);
  pageNumber->setMinimum(1);
  pageSlider = new QSlider(Qt::Horizontal, this);
  pageSlider->setObjectName("ReportWorkspacePageSlider");
  pageSlider->setRange(1, 1);
  pageSlider->setTracking(false);
  pageSlider->setMinimumWidth(100);
  connect(pageSlider, &QSlider::sliderMoved, this, [this](int value)
  {
    pageSlider->setToolTip(tr("Page %1").arg(value));
  });
  connect(pageSlider, &QSlider::valueChanged, this, [this](int value)
  {
    pageNumber->setValue(value);
  });
  zoom = new QSpinBox(this);
  zoom->setRange(1, 300);
  zoom->setSuffix(" %");
  navigation->addWidget(previous);
  navigation->addWidget(pageNumber);
  navigation->addWidget(pageSlider, 1);
  navigation->addWidget(next);
  navigation->addWidget(new QLabel(tr("Zoom"), this));
  navigation->addWidget(zoom);
  for (const auto& mode : {std::string("width"), std::string("page")})
  {
    auto* button = new QPushButton(mode == "width" ? tr("Fit width") : tr("Fit page"), this);
    navigation->addWidget(button);
    connect(button, &QPushButton::clicked, this, [this, mode]()
    {
      dispatch("report preview fit " + mode, true);
    });
  }
  navigation->addStretch();
  outer->addWidget(navigationBar);
  page = new ReportPreviewWidget(this);
  page->set_preview_command([this](const std::string& command)
  {
    dispatch(command, true);
  });
  page->set_position_changed([this](int visible, int percent)
  {
    QSignalBlocker pageGuard(pageNumber), zoomGuard(zoom), sliderGuard(pageSlider);
    if (!pageSlider->isSliderDown())
    {
      pageSlider->setValue(visible + 1);
    }
    if (!pageNumber->hasFocus())
    {
      pageNumber->setValue(visible + 1);
    }
    if (!zoom->hasFocus())
    {
      zoom->setValue(percent);
    }
  });
  outer->addWidget(page, 1);
  connect(pageNumber, &QSpinBox::valueChanged, this, [this](int value)
  {
    dispatch("report preview page " + std::to_string(value), true);
  });
  connect(zoom, &QSpinBox::valueChanged, this, [this](int value)
  {
    dispatch("report preview zoom " + std::to_string(value), true);
  });
  connect(previous, &QPushButton::clicked, this, [this]()
  {
    pageNumber->setValue(pageNumber->value() - 1);
  });
  connect(next, &QPushButton::clicked, this, [this]()
  {
    pageNumber->setValue(pageNumber->value() + 1);
  });
  treeDock = new QDockWidget(tr("Report content"), qobject_cast<QMainWindow*>(host));
  treeDock->setObjectName("ReportContentDock");
  auto callback = [this](const std::string& command, std::string& error)
  {
    const bool result = submit(command, error);
    QTimer::singleShot(0, this, [this]()
    {
      refresh();
    });
    return result;
  };
  tree = new ReportContentTree(treeDock, callback);
  treeDock->setWidget(tree);
  treeDock->hide();
  auto previewCallback = [this](const std::string& command, std::string& error)
  {
    const bool result = silent(command, error);
    QTimer::singleShot(0, this, [this]()
    {
      refresh();
    });
    return result;
  };
  preview = new ReportPreviewWindow(this, callback, previewCallback);
  menu = new QMenu(tr("Report"), this);
  connect(menu->addAction(tr("Open preview / Export")), &QAction::triggered, this, [this]()
  {
    dispatch("report preview");
  });
  refresh();
}

ReportWorkspace::~ReportWorkspace()
{
  delete toolbar.data();
  delete treeDock.data();
}

void ReportWorkspace::dispatch(const std::string& command, bool quiet)
{
  std::string error;
  if (!(quiet ? silent : submit)(command, error))
  {
    PRINT_ERROR("Report command failed: %s\n", error.c_str());
  }
  QTimer::singleShot(0, this, [this]()
  {
    refresh();
  });
}

void ReportWorkspace::choose_source(int index)
{
  if (index < 0 || !sources->itemData(index).isValid())
  {
    return;
  }
  const int entry = sources->itemData(index).toInt();
  if (entry >= 0 && entry < static_cast<int>(entries.size()) && (entries[entry].available || entries[entry].has_report))
  {
    const auto active = application.lock();
    if (active && sameSource(entries[entry].request, active->configuration_request()))
    {
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

void ReportWorkspace::generate()
{
  if (!sources->currentData().isValid())
  {
    return;
  }
  const int index = sources->currentData().toInt();
  if (index < 0 || index >= static_cast<int>(entries.size()) || !entries[index].available)
  {
    return;
  }
  // Profile selection already loads its settings through a command. Generate
  // uses the current configuration, including edits made after loading it.
  dispatch(sourceCommand(entries[index].request, false));
}

void ReportWorkspace::profile_action(const std::string& action)
{
  QString name = profiles->currentIndex() > 0 ? profiles->currentText() : QString();
  if (action == "save")
  {
    bool ok = false;
    name = QInputDialog::getText(this, tr("Save report profile"), tr("Profile name"), QLineEdit::Normal, name, &ok);
    if (!ok)
    {
      return;
    }
  }
  std::string quoted, error;
  if (!name.isEmpty() && ReportCommandText::quote(name, quoted, error))
  {
    profileWorkspace.clear();
    dispatch("report profile " + action + " " + quoted);
  }
}

void ReportWorkspace::refresh()
{
  const auto active = application.lock();
  if (!active)
  {
    return;
  }
  const auto& state = active->session();
  toolbar->set_export_enabled(active->document() != nullptr && !state.configuration_changed && !state.pending_source);
  const auto& request = active->configuration_request();
  std::string error;
  const bool sourcesAvailable = active->available_sources(entries, error);
  // Refresh only reflects Core state. Source commands require explicit activation.
  if (!sourcesAvailable)
  {
    entries.clear();
  }
  {
    QSignalBlocker guard(sources);
    sources->clear();
    std::string group;
    int selected = -1;
    for (std::size_t index = 0; index < entries.size(); ++index)
    {
      const auto& entry = entries[index];
      if (group != entry.workspace_label)
      {
        group = entry.workspace_label;
        sources->addItem(QString::fromStdString(group), -1);
        auto* item = qobject_cast<QStandardItemModel*>(sources->model())->item(sources->count() - 1);
        item->setFlags(Qt::NoItemFlags);
      }
      sources->addItem(QString::fromStdString(entry.label), static_cast<int>(index));
      auto* item = qobject_cast<QStandardItemModel*>(sources->model())->item(sources->count() - 1);
      item->setToolTip(QString::fromStdString(entry.unavailable_reason));
      if (!entry.available && !entry.has_report)
      {
        item->setEnabled(false);
      }
      if (sameSource(entry.request, request))
      {
        selected = sources->count() - 1;
      }
    }
    sources->setCurrentIndex(selected);
  }
  const int sourceIndex = sources->currentData().isValid() ? sources->currentData().toInt() : -1;
  const bool sourceReady = sourcesAvailable && sourceIndex >= 0 && sourceIndex < static_cast<int>(entries.size()) &&
    (entries[sourceIndex].available || entries[sourceIndex].has_report) && sameSource(entries[sourceIndex].request, request);
  const bool canRegenerate = sourceReady && entries[sourceIndex].available;
  generateButton->setEnabled(canRegenerate);
  settingsButton->setEnabled(sourceReady);
  profileButton->setEnabled(sourceReady);
  profiles->setEnabled(sourceReady);
  tree->setEnabled(sourceReady);
  if (!sourceReady)
  {
    QSignalBlocker treeGuard(tree);
    QSignalBlocker profileGuard(profiles);
    tree->clear();
    profiles->clear();
    profileWorkspace.clear();
  }
  updateButton->setEnabled(canRegenerate && active->document() != nullptr && !state.pending_source);
  ReportTemplate definition;
  if (sourceReady && active->describe(definition, error))
  {
    tree->refresh(definition, request.overrides.values.empty() ? definition.defaults : request.overrides);
    if (profileWorkspace != definition.workspace_id)
    {
      std::vector<ReportProfile> available;
      if (active->profile("list", "", available, error))
      {
        QSignalBlocker guard(profiles);
        profiles->clear();
        profiles->addItem(tr("Workspace default"));
        for (const auto& profile : available)
        {
          profiles->addItem(QString::fromStdString(profile.name), QString::fromStdString(profile.id));
        }
        profileWorkspace = definition.workspace_id;
      }
    }
    QSignalBlocker guard(profiles);
    const int selected = profiles->findData(QString::fromStdString(request.profile_id));
    profiles->setCurrentIndex(selected >= 0 ? selected : 0);
  }
  std::shared_ptr<const ReportDocumentLayout> layout;
  if (active->document() && !state.pending_source && active->layout(layout, error))
  {
    page->set_layout(layout);
    preview->set_layout(layout);
    const bool navigate = pageRevision != active->preview_page_revision();
    const bool refit = fitRevision != active->preview_fit_revision();
    pageRevision = active->preview_page_revision();
    fitRevision = active->preview_fit_revision();
    page->set_view(active->preview_page(), active->preview_zoom(), active->preview_mode(), navigate, refit);
    preview->set_view(active->preview_page(), active->preview_zoom(), active->preview_mode(), navigate, refit);
    QSignalBlocker guard(pageNumber), sliderGuard(pageSlider);
    pageSlider->setRange(1, layout->page_count());
    pageNumber->setSuffix(tr(" / %1").arg(layout->page_count()));
    if (!pageSlider->isSliderDown())
    {
      pageSlider->setValue(page->page() + 1);
    }
    pageNumber->setMaximum(layout->page_count());
    pageNumber->setValue(page->page() + 1);
    status->setText(state.configuration_changed ? tr("Settings changed. Update before export; preview shows the last generated report.") :
      tr("%1 pages. Save the project to retain the report.").arg(layout->page_count()));
  }
  else
  {
    {
      QSignalBlocker sliderGuard(pageSlider), numberGuard(pageNumber);
      pageSlider->setRange(1, 1);
      pageNumber->setRange(1, 1);
      pageNumber->setSuffix(QString());
    }
    page->set_layout(nullptr);
    preview->set_layout(nullptr);
    preview->hide();
    status->setText(state.pending_source ? tr("This source has no generated report. Configure its content, then Generate.") :
      QString::fromStdString(error.empty() ? "Choose a source, configure content, then Generate." : error));
  }
  navigationBar->setEnabled(layout != nullptr);
  if (previewWindowRevision != active->preview_window_revision())
  {
    previewWindowRevision = active->preview_window_revision();
    if (layout)
    {
      preview->show();
      preview->raise();
    }
  }
}

void ReportWorkspace::workspace_view(WorkspaceView& view)
{
  view.id = "report";
  view.label = "Report";
  view.icon = icon;
  view.graphicsPage = this;
  view.useNativeNavigation = false;
  view.dockLayout = WorkspaceDockLayout::StackedLeft;
  view.docks = {treeDock};
  view.menuActions = {menu->menuAction()};
  view.toolbars = {toolbar.data()};
  view.activationChanged = [widget = QPointer<ReportWorkspace>(this)](bool active)
  {
    if (active && widget)
    {
      widget->refresh();
    }
  };
}

bool ReportWorkspace::bind_native_toolbar(Claro* host, std::string& error)
{
  return toolbar->bind_native_actions(host, error);
}
