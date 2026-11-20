#include "PostprocessorProperties.hpp"
#include "PostprocessorTopologyCutoffControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Selection/PostprocessorProbeProperties.hpp"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <utility>
#include <QLabel>
#include <type_traits>
#include <variant>

PostprocessorProperties::PostprocessorProperties(std::weak_ptr<PostprocessorApplicationInterface> application,
  QWidget* parent,PostprocessorAppearanceControls::Submit submit) : QWidget(parent), application(application)
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
  topologyCutoff = new PostprocessorTopologyCutoffControls(submit,content);
  contents->addWidget(topologyCutoff);
  probe = new PostprocessorProbeProperties(application,content);
  contents->addWidget(probe);
  appearance = new PostprocessorAppearanceControls(application,std::move(submit),content);
  contents->addWidget(appearance);

  contents->addStretch();
  scroll->setWidget(content);
  layout->addWidget(scroll);
}

void PostprocessorProperties::show_source(const Postprocessor::DataSource* source)
{
  appearance->refresh();
  name->clear();
  origin->clear();

  bool showProbe = false;
  std::size_t frames = 0;
  if (const auto app = application.lock())
  {
    if (source)
    {
      if (const auto data = app->result_data(source->id))
      {
        frames = data->increments.size();
      }
    }
    const auto view = app->session().views.find(app->session().active_view_id);
    showProbe = source && view != app->session().views.end() && view->second.selection.entity &&
      view->second.selection.entity->source_id == source->id;
  }
  topologyCutoff->show_source(source,frames);
  probe->setVisible(showProbe);
  if (showProbe)
  {
    probe->refresh();
  }

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
      if (reference.topology_threshold)
      {
        return QString::fromStdString(reference.application_id) + " topology " + QString::number(reference.job_id);
      }
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

void PostprocessorProperties::set_appearance_targets(const std::vector<Postprocessor::AppearanceTarget>& targets)
{
  appearance->set_targets(targets);
}
