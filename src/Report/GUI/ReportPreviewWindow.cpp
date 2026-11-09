#include "ReportPreviewWindow.hpp"
#include <utility>
#include "ReportPreviewWidget.hpp"
#include "ReportCommandText.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "CubitMessage.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QListWidget>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QSignalBlocker>
#include <map>

ReportPreviewWindow::ReportPreviewWindow(QWidget* parent, std::function<bool(const std::string&, std::string&)> callback,
  std::function<bool(const std::string&, std::string&)> quiet)
  : QDialog(parent), submit(std::move(callback)), silent(std::move(quiet))
{
  setWindowTitle(tr("Report preview"));
  resize(1000, 800);
  auto* outer = new QVBoxLayout(this);
  auto* controls = new QHBoxLayout();
  auto* previous = new QPushButton(tr("Previous"), this);
  auto* next = new QPushButton(tr("Next"), this);
  number = new QSpinBox(this);
  number->setMinimum(1);
  pageSlider = new QSlider(Qt::Horizontal, this);
  pageSlider->setObjectName("ReportPreviewWindowPageSlider");
  pageSlider->setRange(1, 1);
  pageSlider->setTracking(false);
  pageSlider->setMinimumWidth(100);
  connect(pageSlider, &QSlider::sliderMoved, this, [this](int value)
  {
    pageSlider->setToolTip(tr("Page %1").arg(value));
  });
  connect(pageSlider, &QSlider::valueChanged, this, [this](int value)
  {
    number->setValue(value);
  });
  zoom = new QSpinBox(this);
  zoom->setRange(1, 300);
  zoom->setSuffix(" %");
  controls->addWidget(previous);
  controls->addWidget(number);
  controls->addWidget(pageSlider, 1);
  controls->addWidget(next);
  controls->addWidget(new QLabel(tr("Zoom"), this));
  controls->addWidget(zoom);
  for (const std::string mode : {"width", "page"})
  {
    auto* fit = new QPushButton(mode == "width" ? tr("Fit width") : tr("Fit page"), this);
    controls->addWidget(fit);
    connect(fit, &QPushButton::clicked, this, [this, mode]()
    {
      std::string error;
      if (!silent("report preview fit " + mode, error))
      {
        PRINT_ERROR("Cannot fit report preview: %s\n", error.c_str());
      }
    });
  }
  controls->addStretch();
  for (const std::string format : {"pdf", "odt", "both"})
  {
    auto* button = new QPushButton(QString::fromStdString("Export " + format), this);
    controls->addWidget(button);
    connect(button, &QPushButton::clicked, this, [this, format]()
    {
      export_file(format);
    });
  }
  outer->addLayout(controls);
  auto* splitter = new QSplitter(this);
  contents = new QListWidget(splitter);
  page = new ReportPreviewWidget(splitter);
  page->set_preview_command([this](const std::string& command)
  {
    std::string error;
    if (!silent(command, error))
    {
      PRINT_ERROR("Report preview command failed: %s\n", error.c_str());
    }
  });
  page->set_position_changed([this](int visible, int percent)
  {
    QSignalBlocker pageGuard(number), zoomGuard(zoom), sliderGuard(pageSlider);
    if (!pageSlider->isSliderDown())
    {
      pageSlider->setValue(visible + 1);
    }
    if (!number->hasFocus())
    {
      number->setValue(visible + 1);
    }
    if (!zoom->hasFocus())
    {
      zoom->setValue(percent);
    }
  });
  splitter->setStretchFactor(1, 1);
  splitter->setSizes({220, 750});
  outer->addWidget(splitter);
  connect(number, &QSpinBox::valueChanged, this, [this](int value)
  {
    std::string error;
    if (!silent("report preview page " + std::to_string(value), error))
    {
      PRINT_ERROR("Cannot navigate report preview: %s\n", error.c_str());
    }
  });
  connect(previous, &QPushButton::clicked, this, [this]()
  {
    number->setValue(number->value() - 1);
  });
  connect(next, &QPushButton::clicked, this, [this]()
  {
    number->setValue(number->value() + 1);
  });
  connect(zoom, &QSpinBox::valueChanged, this, [this](int value)
  {
    std::string error;
    if (!silent("report preview zoom " + std::to_string(value), error))
    {
      PRINT_ERROR("Cannot zoom report preview: %s\n", error.c_str());
    }
  });
  connect(contents, &QListWidget::itemClicked, this, [this](QListWidgetItem* item)
  {
    number->setValue(item->data(Qt::UserRole).toInt() + 1);
  });
}

ReportPreviewWindow::~ReportPreviewWindow() = default;

void ReportPreviewWindow::set_layout(std::shared_ptr<const ReportDocumentLayout> layout)
{
  if (currentLayout == layout)
  {
    return;
  }
  currentLayout = layout;
  page->set_layout(layout);
  contents->clear();
  QSignalBlocker guard(number), sliderGuard(pageSlider);
  pageSlider->setRange(1, layout ? layout->page_count() : 1);
  number->setSuffix(layout ? tr(" / %1").arg(layout->page_count()) : QString());
  pageSlider->setEnabled(layout != nullptr);
  number->setMaximum(layout ? layout->page_count() : 1);
  if (layout)
  {
    std::map<std::string, std::string> titles;
    std::function<void(const std::vector<ReportSection>&)> collectTitles = [&](const std::vector<ReportSection>& sections)
    {
      for (const auto& section : sections)
      {
        titles[section.id] = section.title;
        collectTitles(section.children);
      }
    };
    collectTitles(layout->report().sections);
    for (const auto& section : layout->contents())
    {
      auto* item = new QListWidgetItem(QString::fromStdString(titles.at(section.first)), contents);
      item->setData(Qt::UserRole, section.second);
    }
  }
}

void ReportPreviewWindow::set_view(int pageNumber, int zoomPercent, const std::string& mode, bool navigate, bool refit)
{
  QSignalBlocker pageGuard(number), sliderGuard(pageSlider);
  QSignalBlocker zoomGuard(zoom);
  page->set_view(pageNumber, zoomPercent, mode, navigate, refit);
  number->setValue(page->page() + 1);
  if (!pageSlider->isSliderDown())
  {
    pageSlider->setValue(page->page() + 1);
  }
  zoom->setValue(page->percent());
}

void ReportPreviewWindow::export_file(const std::string& format)
{
  const QString filter = format == "pdf" ? "PDF (*.pdf)" : format == "odt" ? "OpenDocument text (*.odt)" : "Output base name (*)";
  const QString filename = QFileDialog::getSaveFileName(this, tr("Export report"), QString(), filter);
  if (filename.isEmpty())
  {
    return;
  }
  std::string quoted, error;
  if (!ReportCommandText::quote(filename, quoted, error) || !submit("report export " + format + " " + quoted, error))
  {
    PRINT_ERROR("Cannot export report: %s\n", error.c_str());
  }
}
