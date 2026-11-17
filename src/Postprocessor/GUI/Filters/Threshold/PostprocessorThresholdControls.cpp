#include "PostprocessorThresholdControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/filters/ThresholdCommandText.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "CubitMessage.hpp"
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDoubleValidator>
#include <QLocale>
#include <QTimer>
#include <QScopedValueRollback>
#include <cmath>
#include <algorithm>
#include <utility>

PostprocessorThresholdControls::PostprocessorThresholdControls(
  std::weak_ptr<PostprocessorApplicationInterface> value,
  std::function<void(const QString&)> dispatch,QWidget* parent)
  : QWidget(parent),application(std::move(value)),submit(std::move(dispatch))
{
  auto* form = new QFormLayout(this);
  filters = new QComboBox(this);
  filters->setObjectName("Postprocessor.Threshold.Filters");
  form->addRow(tr("Threshold"),filters);
  name = new QLineEdit(this);
  name->setObjectName("Postprocessor.Threshold.Name");
  form->addRow(tr("Name"),name);
  sourceLabel = new QLabel(this);
  form->addRow(tr("Source"),sourceLabel);
  fields = new QComboBox(this);
  fields->setObjectName("Postprocessor.Threshold.Field");
  form->addRow(tr("Field"),fields);
  components = new QComboBox(this);
  components->setObjectName("Postprocessor.Threshold.Component");
  form->addRow(tr("Component"),components);
  datasets = new QComboBox(this);
  datasets->setObjectName("Postprocessor.Threshold.Dataset");
  form->addRow(tr("Dataset"),datasets);
  unitLabel = new QLabel(this);
  form->addRow(tr("Limit units"),unitLabel);
  lowerEnabled = new QCheckBox(tr("Lower limit"),this);
  upperEnabled = new QCheckBox(tr("Upper limit"),this);
  lower = new QLineEdit("0",this);
  upper = new QLineEdit("1",this);
  lower->setObjectName("Postprocessor.Threshold.Lower");
  upper->setObjectName("Postprocessor.Threshold.Upper");
  lowerEnabled->setObjectName("Postprocessor.Threshold.LowerEnabled");
  upperEnabled->setObjectName("Postprocessor.Threshold.UpperEnabled");
  lowerEnabled->setChecked(true);
  upperEnabled->setChecked(true);
  for (auto* input : {lower,upper})
  {
    auto* validator = new QDoubleValidator(input);
    validator->setLocale(QLocale::c());
    input->setValidator(validator);
    connect(input,&QLineEdit::editingFinished,this,[this]()
    {
      if (!loading && !filters->currentData().toString().isEmpty())
      {
        submit_threshold(false);
      }
    });
  }
  form->addRow(lowerEnabled,lower);
  form->addRow(upperEnabled,upper);
  nodeRule = new QComboBox(this);
  nodeRule->setObjectName("Postprocessor.Threshold.NodeRule");
  nodeRule->addItems({tr("All nodes"),tr("Any node")});
  nodeRule->setToolTip(tr("Keep a whole element when all or any of its nodes match. Invalid samples never match."));
  form->addRow(tr("Nodal matching"),nodeRule);
  enabled = new QCheckBox(tr("Enabled"),this);
  enabled->setObjectName("Postprocessor.Threshold.Enabled");
  visible = new QCheckBox(tr("Show in active view"),this);
  reference = new QCheckBox(tr("Show removed portion"),this);
  form->addRow(enabled);
  form->addRow(visible);
  form->addRow(reference);
  auto* create = new QPushButton(tr("Create threshold"),this);
  create->setObjectName("Postprocessor.Threshold.Create");
  auto* remove = new QPushButton(tr("Remove selected filter"),this);
  form->addRow(create);
  form->addRow(remove);
  connect(create,&QPushButton::clicked,this,[this]()
  {
    submit_threshold(true);
  });
  connect(filters,&QComboBox::activated,this,[this](int)
  {
    if (!filters->currentData().toString().isEmpty())
    {
      send("post filter select "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString()));
    }
    else
    {
      name->clear();
      nodeRule->setCurrentIndex(0);
      refresh();
    }
  });
  connect(fields,&QComboBox::activated,this,[this](int)
  {
    if (!loading)
    {
      refresh_components();
      refresh_units();
      if (!filters->currentData().toString().isEmpty())
      {
        submit_threshold(false);
      }
    }
  });
  connect(components,&QComboBox::activated,this,[this](int)
  {
    if (!loading)
    {
      refresh_units();
      if (!filters->currentData().toString().isEmpty())
      {
        submit_threshold(false);
      }
    }
  });
  connect(datasets,&QComboBox::activated,this,[this](int)
  {
    if (!loading && !filters->currentData().toString().isEmpty())
    {
      submit_threshold(false);
    }
  });
  for (auto* toggle : {lowerEnabled,upperEnabled})
  {
    connect(toggle,&QCheckBox::toggled,this,[this](bool)
    {
      lower->setEnabled(lowerEnabled->isChecked());
      upper->setEnabled(upperEnabled->isChecked());
      if (!loading && !filters->currentData().toString().isEmpty())
      {
        submit_threshold(false);
      }
    });
  }
  connect(nodeRule,&QComboBox::activated,this,[this](int index)
  {
    if (!loading && !filters->currentData().toString().isEmpty())
    {
      QString rule = "all";
      if (index == 1)
      {
        rule = "any";
      }
      send("post threshold nodes "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString())+" "+rule);
    }
  });
  connect(enabled,&QCheckBox::toggled,this,[this](bool checked)
  {
    if (!loading && !filters->currentData().toString().isEmpty())
    {
      QString action = "disable";
      if (checked)
      {
        action = "enable";
      }
      send("post filter "+action+" "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString()));
    }
  });
  connect(visible,&QCheckBox::toggled,this,[this](bool checked)
  {
    if (!loading && !filters->currentData().toString().isEmpty())
    {
      QString action = "hide";
      if (checked)
      {
        action = "show";
      }
      send("post filter "+action+" "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString()));
    }
  });
  connect(reference,&QCheckBox::toggled,this,[this](bool checked)
  {
    if (!loading && !filters->currentData().toString().isEmpty())
    {
      QString state = "off";
      if (checked)
      {
        state = "on";
      }
      send("post threshold reference "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString())+" "+state);
    }
  });
  connect(name,&QLineEdit::editingFinished,this,[this]()
  {
    if (!loading && !filters->currentData().toString().isEmpty() && name->text() != filters->currentText())
    {
      const auto quoted = Postprocessor::plot_quote(name->text().toUtf8().toStdString());
      if (quoted.isEmpty())
      {
        PRINT_ERROR("Cannot quote the threshold name.\n");
        return;
      }
      send("post filter rename "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString())+" "+quoted);
    }
  });
  connect(remove,&QPushButton::clicked,this,[this]()
  {
    if (!filters->currentData().toString().isEmpty())
    {
      send("post filter remove "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString()));
    }
  });
  refresh();
}
PostprocessorThresholdControls::~PostprocessorThresholdControls() = default;
void PostprocessorThresholdControls::send(const QString& command)
{
  if (submit)
  {
    submit(command);
  }
  QTimer::singleShot(0,this,[this]()
  {
    refresh();
  });
}
std::string PostprocessorThresholdControls::source_id() const
{
  const auto app = application.lock();
  if (!app)
  {
    return {};
  }
  const auto found = app->session().filters.find(filters->currentData().toString().toStdString());
  if (found != app->session().filters.end())
  {
    return found->second.input.object_id;
  }
  return app->session().active_source_id;
}
void PostprocessorThresholdControls::refresh_components()
{
  components->clear();
  const int index = fields->currentIndex();
  if (index < 0 || static_cast<std::size_t>(index) >= catalogue.size())
  {
    return;
  }
  const auto& field = catalogue[static_cast<std::size_t>(index)].field;
  if (field.vector_components)
  {
    components->addItem(tr("Magnitude"),QString());
  }
  for (const auto& component : field.components)
  {
    components->addItem(QString::fromStdString(component),QString::fromStdString(component));
  }
  if (!field.vector_components)
  {
    components->setCurrentIndex(components->findData(QString::fromStdString(Postprocessor::preferred_field_component(field))));
  }
  nodeRule->setEnabled(field.association == SimResults::Association::Node);
}
void PostprocessorThresholdControls::refresh_units()
{
  const int index = fields->currentIndex();
  if (index < 0 || static_cast<std::size_t>(index) >= catalogue.size())
  {
    unitLabel->clear();
    return;
  }
  Postprocessor::ThresholdDefinition value;
  value.field.component = components->currentData().toString().toStdString();
  value.field.magnitude = components->currentData().toString().isEmpty();
  std::string error;
  if (Postprocessor::bind_threshold_units(value,catalogue[static_cast<std::size_t>(index)].field,error))
  {
    QString label = tr("Unknown unit");
    if (!value.bounds_unit->unit.empty())
    {
      label = QString::fromStdString(value.bounds_unit->unit);
    }
    unitLabel->setText(label);
  }
  else
  {
    unitLabel->setText(QString::fromStdString(error));
  }
}
void PostprocessorThresholdControls::submit_threshold(bool create)
{
  const auto app = application.lock();
  const int index = fields->currentIndex();
  if (!app || index < 0 || static_cast<std::size_t>(index) >= catalogue.size())
  {
    PRINT_ERROR("Choose an available threshold field.\n");
    return;
  }
  const auto source = app->session().sources.find(source_id());
  if (source == app->session().sources.end())
  {
    PRINT_ERROR("Choose an opened source before creating a threshold.\n");
    return;
  }
  auto value = Postprocessor::ThresholdDefinition{};
  const auto& group = catalogue[static_cast<std::size_t>(index)];
  value.field = Postprocessor::preferred_field_selection(group.field,group.key);
  value.field.component = components->currentData().toString().toStdString();
  value.field.magnitude = value.field.component.empty();
  if (!datasets->currentData().toString().isEmpty())
  {
    value.field.dataset_id = static_cast<std::size_t>(datasets->currentData().toULongLong());
  }
  if (nodeRule->currentIndex() == 1)
  {
    value.node_rule = Postprocessor::ThresholdNodeRule::Any;
  }
  const auto read = [](QLineEdit* input,std::optional<double>& output) -> bool
  {
    bool valid = false;
    const double number = QLocale::c().toDouble(input->text(),&valid);
    if (!valid || !std::isfinite(number))
    {
      PRINT_ERROR("Threshold limits must be finite numbers.\n");
      return false;
    }
    output = number;
    return true;
  };
  if ((lowerEnabled->isChecked() && !read(lower,value.lower)) ||
      (upperEnabled->isChecked() && !read(upper,value.upper)))
  {
    return;
  }
  std::string error;
  Postprocessor::ResultFieldSelector selector;
  if (!app->readable_field(source->first,value.field,selector,error))
  {
    PRINT_ERROR("Cannot describe the threshold field: %s\n",error.c_str());
    return;
  }
  std::string filterName = filters->currentText().toUtf8().toStdString();
  QString action = "configure";
  if (create)
  {
    action = "create";
    filterName = name->text().toUtf8().toStdString();
    if (filterName.empty())
    {
      filterName = app->suggested_filter_name(Postprocessor::FilterKind::Threshold);
    }
  }
  const auto quoted = Postprocessor::plot_quote(filterName);
  const auto sourceName = Postprocessor::plot_quote(source->second.name);
  const auto settings = Postprocessor::threshold_command_text(selector,value);
  if (quoted.isEmpty() || sourceName.isEmpty() || settings.isEmpty())
  {
    PRINT_ERROR("Cannot quote the threshold command arguments.\n");
    return;
  }
  QString command = "post threshold "+action+" "+quoted;
  if (create)
  {
    command += " source "+sourceName;
  }
  send(command+" "+settings);
}
void PostprocessorThresholdControls::refresh()
{
  if (loading)
  {
    return;
  }
  const auto app = application.lock();
  if (!app)
  {
    setEnabled(false);
    return;
  }
  const QScopedValueRollback<bool> guard(loading,true);
  auto selected = filters->currentData().toString();
  const auto& state = app->session();
  const auto active = state.representations.find(state.active_representation_id);
  if (active != state.representations.end() && lastActive != state.active_representation_id &&
      active->second.input.owner == Postprocessor::OutputOwner::Feature)
  {
    const auto filter = state.filters.find(active->second.input.object_id);
    if (filter != state.filters.end() && filter->second.kind == Postprocessor::FilterKind::Threshold)
    {
      selected = QString::fromStdString(filter->first);
    }
  }
  lastActive = state.active_representation_id;
  filters->clear();
  filters->addItem(tr("New threshold..."),QString());
  for (const auto& entry : state.filters)
  {
    if (entry.second.kind == Postprocessor::FilterKind::Threshold)
    {
      filters->addItem(QString::fromStdString(entry.second.name),QString::fromStdString(entry.first));
    }
  }
  const int selectedIndex = filters->findData(selected);
  filters->setCurrentIndex(std::max(0,selectedIndex));
  const auto found = state.filters.find(filters->currentData().toString().toStdString());
  const bool existing = found != state.filters.end();
  const auto source = source_id();
  const auto sourceEntry = state.sources.find(source);
  sourceLabel->clear();
  if (sourceEntry != state.sources.end())
  {
    sourceLabel->setText(QString::fromStdString(sourceEntry->second.name));
  }
  auto fieldKey = fields->currentData().toString();
  auto component = components->currentData().toString();
  std::optional<std::size_t> dataset;
  if (!datasets->currentData().toString().isEmpty())
  {
    dataset = static_cast<std::size_t>(datasets->currentData().toULongLong());
  }
  if (existing)
  {
    fieldKey = QString::fromStdString(found->second.threshold.field.group_key);
    component = QString::fromStdString(found->second.threshold.field.component);
    dataset = found->second.threshold.field.dataset_id;
  }
  else if (fields->count() == 0 && active != state.representations.end() && active->second.coloring.field)
  {
    fieldKey = QString::fromStdString(active->second.coloring.field->group_key);
    component = QString::fromStdString(active->second.coloring.field->component);
    dataset = active->second.coloring.field->dataset_id;
  }
  catalogue = app->field_groups(source,std::nullopt);
  catalogue.erase(std::remove_if(catalogue.begin(),catalogue.end(),[](const auto& group)
  {
    return group.field.association == SimResults::Association::Table;
  }),catalogue.end());
  fields->clear();
  for (const auto& group : catalogue)
  {
    fields->addItem(QString::fromStdString(group.field.name+" ["+group.field.source+", "+
      Postprocessor::field_association_name(group.field.association)+", "+group.field.set_name+"]"),QString::fromStdString(group.key));
  }
  const int fieldIndex = fields->findData(fieldKey);
  if (existing && fieldIndex < 0)
  {
    fields->setCurrentIndex(-1);
  }
  else
  {
    fields->setCurrentIndex(std::max(0,fieldIndex));
  }
  refresh_components();
  const int componentIndex = components->findData(component);
  if (componentIndex >= 0)
  {
    components->setCurrentIndex(componentIndex);
  }
  refresh_units();
  datasets->clear();
  datasets->addItem(tr("All compatible datasets"),QString());
  for (const auto& metadata : app->mesh_datasets(source))
  {
    datasets->addItem(QString::fromStdString(metadata.name),QString::number(static_cast<qulonglong>(metadata.id)));
    if (existing && !found->second.threshold.dataset_kind.empty() &&
        metadata.kind == found->second.threshold.dataset_kind && metadata.name == found->second.threshold.dataset_name &&
        metadata.model_id == found->second.threshold.dataset_model)
    {
      dataset = metadata.id;
    }
  }
  if (dataset)
  {
    datasets->setCurrentIndex(std::max(0,datasets->findData(QString::number(static_cast<qulonglong>(*dataset)))));
  }
  for (auto* toggle : {enabled,visible,reference})
  {
    toggle->setEnabled(existing);
  }
  enabled->setChecked(existing && found->second.enabled);
  reference->setChecked(existing && found->second.reference_visible);
  bool shown = false;
  if (existing)
  {
    name->setText(QString::fromStdString(found->second.name));
    const auto& threshold = found->second.threshold;
    lowerEnabled->setChecked(threshold.lower.has_value());
    upperEnabled->setChecked(threshold.upper.has_value());
    if (threshold.lower)
    {
      lower->setText(QString::number(*threshold.lower,'g',17));
    }
    if (threshold.upper)
    {
      upper->setText(QString::number(*threshold.upper,'g',17));
    }
    nodeRule->setCurrentIndex(threshold.node_rule == Postprocessor::ThresholdNodeRule::All ? 0 : 1);
    for (const auto& entry : state.representations)
    {
      shown = shown || (entry.second.view_id == state.active_view_id && entry.second.visible &&
        entry.second.input.owner == Postprocessor::OutputOwner::Feature && entry.second.input.object_id == found->first);
    }
  }
  else
  {
    name->setPlaceholderText(QString::fromStdString(app->suggested_filter_name(Postprocessor::FilterKind::Threshold)));
  }
  lower->setEnabled(lowerEnabled->isChecked());
  upper->setEnabled(upperEnabled->isChecked());
  visible->setChecked(shown);
}
