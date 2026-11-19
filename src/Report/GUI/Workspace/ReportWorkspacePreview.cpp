#include "ReportWorkspace.hpp"
#include "Report/Interfaces/ReportApplicationInterface.hpp"
#include "Report/Core/ReportSession.hpp"
#include "Report/GUI/Content/ReportContentTree.hpp"
#include "Report/GUI/Content/ReportOutlineTree.hpp"
#include "Report/GUI/Preview/ReportPreviewWidget.hpp"
#include "Report/GUI/Preview/ReportPreviewWindow.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include <QDockWidget>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QSignalBlocker>
#include <QTimer>

void ReportWorkspace::create_preview_controls(QVBoxLayout* outer)
{
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
  connect(pageSlider, &QSlider::sliderMoved, this, [this](int value) {
    pageSlider->setToolTip(tr("Page %1").arg(value));
  });
  connect(pageSlider, &QSlider::valueChanged, this, [this](int value) {
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
  for (const auto& mode : {std::string("width"), std::string("page")}) {
    QString caption = tr("Fit page");
    if (mode == "width") {
      caption = tr("Fit width");
    }
    auto* button = new QPushButton(caption, this);
    navigation->addWidget(button);
    connect(button, &QPushButton::clicked, this, [this, mode]() {
      dispatch("report preview fit " + mode, true);
    });
  }
  navigation->addStretch();
  outer->addWidget(navigationBar);
  page = new ReportPreviewWidget(this);
  page->set_preview_command([this](const std::string& command) {
    dispatch(command, true);
  });
  page->set_position_changed([this](int visible, int percent) {
    QSignalBlocker pageGuard(pageNumber), zoomGuard(zoom), sliderGuard(pageSlider);
    if (!pageSlider->isSliderDown()) {
      pageSlider->setValue(visible + 1);
    }
    if (!pageNumber->hasFocus()) {
      pageNumber->setValue(visible + 1);
    }
    if (!zoom->hasFocus()) {
      zoom->setValue(percent);
    }
  });
  outer->addWidget(page, 1);
  connect(pageNumber, &QSpinBox::valueChanged, this, [this](int value) {
    dispatch("report preview page " + std::to_string(value), true);
  });
  connect(zoom, &QSpinBox::valueChanged, this, [this](int value) {
    dispatch("report preview zoom " + std::to_string(value), true);
  });
  connect(previous, &QPushButton::clicked, this, [this]() {
    pageNumber->setValue(pageNumber->value() - 1);
  });
  connect(next, &QPushButton::clicked, this, [this]() {
    pageNumber->setValue(pageNumber->value() + 1);
  });
}

void ReportWorkspace::create_content_and_preview(QWidget* host)
{
  treeDock = new QDockWidget(tr("Report content"), qobject_cast<QMainWindow*>(host));
  treeDock->setObjectName("ReportContentDock");
  auto callback = [this](const std::string& command, std::string& error) {
    const bool result = submit(command, error);
    QTimer::singleShot(0, this, [this]() {
      refresh();
    });
    return result;
  };
  contentTabs = new QTabWidget(treeDock);
  contentTabs->setObjectName("ReportContentTabs");
  tree = new ReportContentTree(contentTabs, callback);
  outline = new ReportOutlineTree(contentTabs, [this](int pageNumber)
  {
    dispatch("report preview page " + std::to_string(pageNumber), true);
  });
  contentTabs->addTab(tree, tr("Content"));
  contentTabs->addTab(outline, tr("Outline"));
  treeDock->setWidget(contentTabs);
  treeDock->hide();
  auto previewCallback = [this](const std::string& command, std::string& error) {
    const bool result = silent(command, error);
    QTimer::singleShot(0, this, [this]() {
      refresh();
    });
    return result;
  };
  preview = new ReportPreviewWindow(this, callback, previewCallback);
}

void ReportWorkspace::refresh_preview(const ReportApplicationInterface& active, const std::string& error)
{
  const auto& state = active.session();
  auto outlinedDocument = active.document();
  if (state.pending_source)
  {
    outlinedDocument.reset();
  }
  // Counts become available before the expensive pagination begins.
  outline->refresh(outlinedDocument);
  const auto layout = active.prepared_layout();
  if (layout && !state.pending_source) {
    outline->refresh(outlinedDocument, &layout->contents(), layout->page_count());
    page->set_layout(layout);
    preview->set_layout(layout);
    const bool navigate = pageRevision != active.preview_page_revision();
    const bool refit = fitRevision != active.preview_fit_revision();
    pageRevision = active.preview_page_revision();
    fitRevision = active.preview_fit_revision();
    page->set_view(active.preview_page(), active.preview_zoom(), active.preview_mode(), navigate, refit);
    preview->set_view(active.preview_page(), active.preview_zoom(), active.preview_mode(), navigate, refit);
    QSignalBlocker guard(pageNumber), sliderGuard(pageSlider);
    pageSlider->setRange(1, layout->page_count());
    pageNumber->setSuffix(tr(" / %1").arg(layout->page_count()));
    if (!pageSlider->isSliderDown()) {
      pageSlider->setValue(page->page() + 1);
    }
    pageNumber->setMaximum(layout->page_count());
    pageNumber->setValue(page->page() + 1);
    if (state.configuration_changed) {
      status->setText(tr("Settings changed. Update before export; preview shows the last generated report."));
    } else {
      status->setText(tr("%1 pages. Save the project to retain the report.").arg(layout->page_count()));
    }
  } else {
    {
      QSignalBlocker sliderGuard(pageSlider), numberGuard(pageNumber);
      pageSlider->setRange(1, 1);
      pageNumber->setRange(1, 1);
      pageNumber->setSuffix(QString());
    }
    page->set_layout(nullptr);
    preview->set_layout(nullptr);
    preview->hide();
    if (state.pending_source) {
      status->setText(tr("This source has no generated report. Configure its content, then Generate."));
    } else if (active.document() && error.empty()) {
      status->setText(tr("Report generated. Use Open preview / Export to prepare its preview."));
    } else if (error.empty()) {
      status->setText(tr("Choose a source, configure content, then Generate."));
    } else {
      status->setText(QString::fromStdString(error));
    }
  }
  navigationBar->setEnabled(layout != nullptr);
  if (previewWindowRevision != active.preview_window_revision()) {
    previewWindowRevision = active.preview_window_revision();
    if (layout) {
      preview->show();
      preview->raise();
    }
  }
}
