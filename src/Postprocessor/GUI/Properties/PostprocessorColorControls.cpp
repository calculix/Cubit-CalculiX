#include "PostprocessorColorControls.hpp"
#include "Postprocessor/Commands/FieldCommandText.hpp"
#include <QComboBox>
#include <utility>
#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QStandardItemModel>
#include <algorithm>
#include "CubitMessage.hpp"
namespace
{
  QString quote(const std::string& value)
  {
    const auto text = QString::fromUtf8(value.c_str());
    const QChar delimiter = text.contains('"') ? QChar(0x27) : QChar('"');
    if (text.isEmpty() || text.contains(delimiter))
    {
      return {};
    }
    for (QChar c : text)
    {
      if (c.unicode() < 32 || c.unicode() == 127)
      {
        return {};
      }
    }
    return delimiter + text + delimiter;
  }
}
PostprocessorColorControls::PostprocessorColorControls(std::weak_ptr<PostprocessorApplicationInterface> active,
  std::function<void(const QString&)> callback, QWidget* parent)
  : QWidget(parent), application(std::move(active)), submit(std::move(callback))
{
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->addWidget(new QLabel(tr("Field"),this));
  fields = new QComboBox(this);
  fields->setObjectName("Postprocessor.ColorField");
  fields->setMinimumContentsLength(14);
  fields->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  fields->setToolTip(tr("Result field for the active source in the active view"));
  layout->addWidget(fields);
  layout->addWidget(new QLabel(tr("Component"),this));
  components = new QComboBox(this);
  components->setToolTip(tr("Published component or declared vector magnitude"));
  layout->addWidget(components);
  connect(fields,&QComboBox::activated,this,[this](int i)
  {
    choose_field(i);
    publish();
  });
  connect(components,&QComboBox::activated,this,[this](int)
  {
    publish();
  });
  refresh();
}
void PostprocessorColorControls::choose_field(int index)
{
  const QSignalBlocker blocker(components);
  components->clear();
  const int entry = fields->itemData(index).toInt();
  if (entry < 0 || static_cast<std::size_t>(entry) >= catalogue.size())
  {
    components->setEnabled(false);
    return;
  }
  const auto& field = catalogue[static_cast<std::size_t>(entry)].field;
  if (field.vector_components)
  {
    components->addItem(tr("Magnitude"),true);
  }
  for (const auto& name : field.components)
  {
    components->addItem(QString::fromUtf8(name.c_str()),false);
  }
  components->setEnabled(components->count() > 0);
}
void PostprocessorColorControls::publish()
{
  const auto sourceArg = quote(source);
  if (sourceArg.isEmpty())
  {
    return;
  }
  const int entry = fields->currentData().toInt();
  if (entry == -1)
  {
    submit("post color solid source " + sourceArg);
    return;
  }
  if (entry < 0 || static_cast<std::size_t>(entry) >= catalogue.size() || components->currentIndex() < 0)
  {
    return;
  }
  const auto& field = catalogue[static_cast<std::size_t>(entry)].field;
  const auto active = application.lock();
  if (!active)
  {
    return;
  }
  std::string sourceId, error;
  for (const auto& entry : active->session().sources)
  {
    if (entry.second.name == source)
    {
      sourceId = entry.first;
      break;
    }
  }
  const bool magnitude = components->currentData().toBool();
  const auto& group = catalogue[static_cast<std::size_t>(entry)];
  Postprocessor::FieldSelection selected{std::nullopt,field.array_name,field.association,
    magnitude ? std::string{} : components->currentText().toUtf8().toStdString(),magnitude,group.key};
  Postprocessor::ResultFieldSelector selector;
  if (!active->readable_field(sourceId,selected,selector,error))
  {
    PRINT_ERROR("Cannot journal result field: %s\n",error.c_str());
    return;
  }
  const auto fieldArg = Postprocessor::field_command_text(selector);
  const auto componentArg = components->currentData().toBool() ? QStringLiteral("magnitude") : quote(components->currentText().toUtf8().toStdString());
  if (fieldArg.isEmpty() || componentArg.isEmpty())
  {
    PRINT_ERROR("This field/component cannot be represented as a quoted command argument.\n");
    return;
  }
  submit("post color " + fieldArg + " component " + componentArg + " source " + sourceArg);
}
void PostprocessorColorControls::refresh()
{
  const QSignalBlocker blocker(fields);
  fields->clear();
  catalogue.clear();
  source.clear();
  fields->addItem(tr("Solid color"),-1);
  const auto active = application.lock();
  std::optional<Postprocessor::ResultColoring> coloring;
  if (active)
  {
    const auto& session = active->session();
    const auto item = session.sources.find(session.active_source_id);
    if (item != session.sources.end())
    {
      source = item->second.name;
      const auto data = active->result_data(item->first);
      for (const auto& entry : session.representations)
      {
        const auto& display = entry.second;
        if (active->display_matches_source(display,item->first,session.active_view_id) && data && display.increment_index < data->increments.size())
        {
          catalogue = active->field_groups(item->first,display.increment_index);
          coloring = display.coloring;
          break;
        }
      }
    }
  }
  int selected = 0;
  for (std::size_t i = 0; i < catalogue.size(); ++i)
  {
    const auto& group = catalogue[i];
    const auto& field = group.field;
    if (field.association == SimResults::Association::Table || field.components.empty())
    {
      continue;
    }
    QString label = QString::fromUtf8(field.name.c_str());
    const auto count = std::count_if(catalogue.begin(),catalogue.end(),[&field](const Postprocessor::ResultFieldGroup& other)
    {
      return other.field.name == field.name;
    });
    if (count > 1)
    {
      QString association;
      switch (field.association)
      {
        case SimResults::Association::Node: association = tr("Node"); break;
        case SimResults::Association::Element: association = tr("Element"); break;
        case SimResults::Association::IntegrationPoint: association = tr("Integration point"); break;
        case SimResults::Association::Table: association = tr("Table"); break;
      }
      label += " [" + association;
      if (!field.source.empty())
      {
        label += " / " + QString::fromUtf8(field.source.c_str());
      }
      if (!field.set_name.empty())
      {
        label += " / " + QString::fromUtf8(field.set_name.c_str());
      }
      label += "]";
    }
    if (!group.available)
    {
      label += tr(" (unavailable)");
    }
    fields->addItem(label,static_cast<int>(i));
    if (auto* model = qobject_cast<QStandardItemModel*>(fields->model()))
    {
      model->item(fields->count()-1)->setEnabled(group.available);
    }
    if (coloring && coloring->field && coloring->field->array_name == field.array_name &&
        (!coloring->field->dataset_id || std::find(group.datasets.begin(),group.datasets.end(),*coloring->field->dataset_id) != group.datasets.end()) &&
        (coloring->field->group_key.empty() || coloring->field->group_key == group.key) && coloring->field->association == field.association)
    {
      selected = fields->count()-1;
    }
  }
  if (coloring && coloring->field && selected == 0)
  {
    fields->addItem(tr("Selected field unavailable"),-2);
    selected = fields->count()-1;
  }
  fields->setCurrentIndex(selected);
  fields->setEnabled(!source.empty() && coloring.has_value());
  choose_field(selected);
  if (coloring && coloring->field)
  {
    for (int i = 0; i < components->count(); ++i)
    {
      if ((coloring->field->magnitude && components->itemData(i).toBool()) ||
          (!coloring->field->magnitude && !components->itemData(i).toBool() &&
            components->itemText(i).toUtf8().toStdString() == coloring->field->component))
      {
        components->setCurrentIndex(i);
        break;
      }
    }
  }
}
void PostprocessorColorControls::apply_field(const QString& name, std::size_t field_index)
{
  const auto active = application.lock();
  if (!active)
  {
    return;
  }
  const auto& session = active->session();
  for (const auto& entry : session.sources)
  {
    if (QString::fromUtf8(entry.second.name.c_str()) != name)
    {
      continue;
    }
    const auto data = active->result_data(entry.first);
    for (const auto& item : session.representations)
    {
      const auto& display = item.second;
      if (!active->display_matches_source(display,entry.first,session.active_view_id) || !data ||
          display.increment_index >= data->increments.size())
      {
        continue;
      }
      const auto available = active->field_groups(entry.first,display.increment_index);
      if (field_index >= available.size())
      {
        return;
      }
      const auto& group = available[field_index];
      const auto& field = group.field;
      if (!group.available)
      {
        return;
      }
      if (field.association == SimResults::Association::Table || field.components.empty())
      {
        return;
      }
      const bool magnitude = field.vector_components.has_value();
      Postprocessor::FieldSelection selected{std::nullopt,field.array_name,field.association,
        magnitude ? std::string{} : field.components.front(),magnitude,group.key};
      Postprocessor::ResultFieldSelector selector;
      std::string error;
      if (!active->readable_field(entry.first,selected,selector,error))
      {
        PRINT_ERROR("Cannot journal result field: %s\n",error.c_str());
        return;
      }
      const auto fieldArg = Postprocessor::field_command_text(selector);
      const auto sourceArg = quote(entry.second.name);
      const auto componentArg = field.vector_components ? QStringLiteral("magnitude") : quote(field.components.front());
      if (fieldArg.isEmpty() || sourceArg.isEmpty() || componentArg.isEmpty())
      {
        PRINT_ERROR("The field/source/component cannot be represented as a quoted command argument.\n");
        return;
      }
      submit("post color " + fieldArg + " component " + componentArg + " source " + sourceArg);
      return;
    }
  }
}
