#include "PostprocessorProperties.hpp"
#include "PostprocessorDisplayProperties.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorProbeProperties.hpp"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <utility>
#include <QLabel>
#include <type_traits>
#include <variant>

PostprocessorProperties::PostprocessorProperties(std::weak_ptr<PostprocessorApplicationInterface> application,
  std::function<bool(const std::string&,std::string&)> submit, QWidget* parent) : QWidget(parent)
{
  setObjectName("Postprocessor.Properties");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  auto* scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  auto* content = new QWidget(scroll);
  auto* contents = new QVBoxLayout(content);
  auto* information = new QFormLayout;
  name = new QLabel(content);
  origin = new QLabel(content);

  for (auto* label : {name, origin})
  {
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  }

  information->addRow(tr("Name"),name);
  information->addRow(tr("Source"),origin);
  contents->addLayout(information);
  probe = new PostprocessorProbeProperties(application,content);
  contents->addWidget(probe);
  display = new PostprocessorDisplayProperties(std::move(application),std::move(submit),content);
  contents->addWidget(display);
  contents->addStretch();
  scroll->setWidget(content);
  layout->addWidget(scroll);
}

void PostprocessorProperties::show_source(const Postprocessor::DataSource* source)
{
  name->clear();
  origin->clear();
  display->refresh();
  probe->refresh();

  if (source == nullptr)
  {
    return;
  }

  name->setText(QString::fromUtf8(source->name.c_str()));
  origin->setText(std::visit([](const auto& reference) -> QString
  {
    using Reference = std::decay_t<decltype(reference)>;

    if constexpr (std::is_same_v<Reference, Postprocessor::JobSourceReference>)
    {
      return QString::fromStdString(reference.application_id) + " job " + QString::number(reference.job_id);
    }
    else if constexpr (std::is_same_v<Reference, Postprocessor::FileSourceReference>)
    {
      QString result = QString::fromStdString(reference.provider_id);

      for (const auto& path : reference.paths)
      {
        result += "\n" + QString::fromUtf8(path.c_str());
      }

      return result;
    }
    else
    {
      return tr("Cubit geometry");
    }
  }, source->reference));
}
