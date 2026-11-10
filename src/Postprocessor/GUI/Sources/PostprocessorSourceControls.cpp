#include "PostprocessorSourceControls.hpp"
#include <QComboBox>
#include <QToolButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QSignalBlocker>
#include <QStyle>
#include <QShowEvent>
namespace
{
  class SourceCombo : public QComboBox
  {
  public:
    using QComboBox::QComboBox;
    std::function<void()> beforePopup;
    void showPopup() override
    {
      if (beforePopup)
      {
        beforePopup();
      }
      QComboBox::showPopup();
    }
  };
}
PostprocessorSourceControls::PostprocessorSourceControls(Catalogue query,
  std::function<void(const SimResults::ResultJobEntry&)> addJob,
  std::function<void()> importFiles, QWidget* parent) : QWidget(parent), catalogue(std::move(query))
{
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(4);
  layout->addWidget(new QLabel(tr("Source"),this));
  auto* combo = new SourceCombo(this);
  sources = combo;
  sources->setObjectName("Postprocessor.SourceSelector");
  sources->setPlaceholderText(tr("Choose a job"));
  sources->setMinimumWidth(180);
  sources->setMaximumWidth(320);
  sources->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  sources->setMinimumContentsLength(20);
  combo->beforePopup = [this]()
  {
    refresh();
  };
  layout->addWidget(sources);
  add = new QToolButton(this);
  add->setObjectName("Postprocessor.AddJob");
  add->setText("+");
  add->setToolTip(tr("Add the selected job results"));
  add->setEnabled(false);
  layout->addWidget(add);
  auto* import = new QToolButton(this);
  importButton = import;
  import->setObjectName("Postprocessor.ImportResults");
  import->setIcon(style()->standardIcon(QStyle::SP_DialogOpenButton));
  import->setToolTip(tr("Import CalculiX or VTK result files"));
  layout->addWidget(import);
  connect(import,&QToolButton::clicked,this,[importFiles]()
  {
    importFiles();
  });
  connect(sources,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int)
  {
    const int entry = sources->currentData().isValid() ? sources->currentData().toInt() : -1;
    add->setEnabled(entry >= 0 && entry < static_cast<int>(entries.size()) && entries[entry].available);
  });
  connect(add,&QToolButton::clicked,this,[this,addJob]()
  {
    const int entry = sources->currentData().isValid() ? sources->currentData().toInt() : -1;
    if (entry >= 0 && entry < static_cast<int>(entries.size()) && entries[entry].available)
    {
      const auto selected = entries[entry];
      addJob(selected);
    }
  });
}
void PostprocessorSourceControls::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  refresh();
}
void PostprocessorSourceControls::refresh()
{
  std::string application;
  int job = -1;
  const int previous = sources->currentData().isValid() ? sources->currentData().toInt() : -1;
  if (previous >= 0 && previous < static_cast<int>(entries.size()))
  {
    application = entries[previous].reference.application_id;
    job = entries[previous].reference.job_id;
  }
  std::string error;
  std::vector<SimResults::ResultJobEntry> current;
  const bool available = catalogue && catalogue(current,error);
  QSignalBlocker guard(sources);
  sources->clear();
  entries = available ? std::move(current) : std::vector<SimResults::ResultJobEntry>{};
  auto* model = qobject_cast<QStandardItemModel*>(sources->model());
  int selected = -1;
  std::string group;
  for (std::size_t index = 0; index < entries.size(); ++index)
  {
    const auto& entry = entries[index];
    if (entry.reference.application_id != group)
    {
      group = entry.reference.application_id;
      sources->addItem(QString::fromUtf8(entry.application_name.c_str()),-1);
      model->item(sources->count()-1)->setEnabled(false);
    }
    sources->addItem(QString::fromUtf8(entry.job_name.c_str())+tr(" — Job %1").arg(entry.reference.job_id),static_cast<int>(index));
    auto* item = model->item(sources->count()-1);
    item->setEnabled(entry.available);
    item->setToolTip(QString::fromUtf8(entry.unavailable_reason.c_str()));
    if (entry.reference.application_id == application && entry.reference.job_id == job)
    {
      selected = sources->count()-1;
    }
  }
  sources->setCurrentIndex(selected);
  sources->setToolTip(available ? tr("Choose a workspace job, then click Add") : QString::fromUtf8(error.c_str()));
  const int index = sources->currentData().isValid() ? sources->currentData().toInt() : -1;
  add->setEnabled(index >= 0 && index < static_cast<int>(entries.size()) && entries[index].available);
}

void PostprocessorSourceControls::set_import_icon(const QIcon& icon)
{
  if (!icon.isNull())
  {
    importButton->setIcon(icon);
  }
}
