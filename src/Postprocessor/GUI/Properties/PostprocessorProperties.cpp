#include "PostprocessorProperties.hpp"

#include <QFormLayout>
#include <QLabel>
#include <type_traits>
#include <variant>

PostprocessorProperties::PostprocessorProperties(QWidget* parent) : QWidget(parent)
{
  setObjectName("Postprocessor.Properties");
  auto* layout = new QFormLayout(this);
  name = new QLabel(this);
  origin = new QLabel(this);

  for (auto* label : {name, origin})
  {
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  }

  layout->addRow(tr("Name"), name);
  layout->addRow(tr("Source"), origin);
}

void PostprocessorProperties::show_source(const Postprocessor::DataSource* source)
{
  name->clear();
  origin->clear();

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
