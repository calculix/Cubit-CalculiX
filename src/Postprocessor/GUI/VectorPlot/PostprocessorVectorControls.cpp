#include "PostprocessorVectorControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/representations/ResultVectors.hpp"
#include "Postprocessor/Commands/results/VectorCommandText.hpp"
#include "CubitMessage.hpp"
#include "Postprocessor/Commands/FieldCommandText.hpp"
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QScopedValueRollback>
#include <utility>
PostprocessorVectorControls::PostprocessorVectorControls(std::weak_ptr<PostprocessorApplicationInterface> active,
  Submit dispatcher,QWidget* parent) : QWidget(parent),application(std::move(active)),submit(std::move(dispatcher))
{
  setObjectName("Postprocessor.VectorControls");
  auto* form = new QFormLayout(this);
  auto* panel = this;
  enabled = new QCheckBox(tr("Show vectors"),this);
  enabled->setObjectName("Postprocessor.Vector.Enabled");
  enabled->setToolTip(tr("Show vector arrows for the selected field"));
  form->addRow(enabled);
  fields = new QComboBox(this);
  fields->setObjectName("Postprocessor.Vector.Field");
  form->addRow(tr("Field"),fields);
  length = new QComboBox(panel);
  length->addItems({tr("Proportional"),tr("Uniform")});
  form->addRow(tr("Arrow length"),length);
  scale = new QDoubleSpinBox(panel);
  scale->setObjectName("Postprocessor.Vector.Scale");
  scale->setRange(0.001,1000);
  scale->setDecimals(3);
  scale->setKeyboardTracking(false);
  form->addRow(tr("Scale"),scale);
  samples = new QComboBox(panel);
  samples->addItems({tr("Surface"),tr("All")});
  form->addRow(tr("Samples"),samples);
  maximum = new QSpinBox(panel);
  maximum->setObjectName("Postprocessor.Vector.Maximum");
  maximum->setRange(0,1000000);
  maximum->setSpecialValueText(tr("All"));
  maximum->setKeyboardTracking(false);
  form->addRow(tr("Maximum arrows"),maximum);
  stride = new QSpinBox(panel);
  stride->setRange(1,1000000);
  stride->setKeyboardTracking(false);
  form->addRow(tr("Every Nth sample"),stride);
  color = new QComboBox(panel);
  color->addItems({tr("Magnitude"),tr("Solid")});
  form->addRow(tr("Color"),color);
  auto* legend = new QPushButton(tr("Vector legend properties..."),this);
  form->addRow(legend);
  connect(legend,&QPushButton::clicked,this,[this]()
  {
    const auto command = Postprocessor::vector_command_text("on",source);
    const auto suffix = command.indexOf(" source ");
    if (suffix < 0)
    {
      return;
    }
    std::string error;
    if (submit)
    {
      submit(("post properties vectorlegend"+command.mid(suffix)).toStdString(),error);
    }
  });
  connect(enabled,&QCheckBox::clicked,this,[this](bool on)
  {
    QString operation = "off";
    if (on)
    {
      operation = "on";
    }
    send(operation);
  });
  connect(fields,qOverload<int>(&QComboBox::activated),this,[this](int row)
  {
    if (refreshing || row < 0 || static_cast<std::size_t>(row) >= selectors.size())
    {
      return;
    }
    send(Postprocessor::field_command_text(selectors[static_cast<std::size_t>(row)]));
  });
  connect(length,qOverload<int>(&QComboBox::activated),this,[this](int row)
  {
    QString operation = "length proportional";
    if (row == 1)
    {
      operation = "length uniform";
    }
    send(operation);
  });
  connect(samples,qOverload<int>(&QComboBox::activated),this,[this](int row)
  {
    QString operation = "samples surface";
    if (row == 1)
    {
      operation = "samples all";
    }
    send(operation);
  });
  connect(color,qOverload<int>(&QComboBox::activated),this,[this](int row)
  {
    QString operation = "color magnitude";
    if (row == 1)
    {
      operation = "color solid";
    }
    send(operation);
  });
  connect(scale,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[this](double value)
  {
    send("scale "+QString::number(value,'g',17));
  });
  connect(maximum,qOverload<int>(&QSpinBox::valueChanged),this,[this](int value)
  {
    send("density maximum "+QString::number(value));
  });
  connect(stride,qOverload<int>(&QSpinBox::valueChanged),this,[this](int value)
  {
    send("density every "+QString::number(value));
  });
}
void PostprocessorVectorControls::send(const QString& operation)
{
  if (refreshing)
  {
    return;
  }
  const auto command = Postprocessor::vector_command_text(operation,source);
  std::string error;
  if (command.isEmpty() || !submit || !submit(command.toUtf8().toStdString(),error))
  {
    PRINT_ERROR("Cannot change vector plot: %s\n",error.c_str());
  }
  refresh();
}
void PostprocessorVectorControls::refresh()
{
  const QScopedValueRollback<bool> guard(refreshing,true);
  fields->clear();
  selectors.clear();
  source.clear();
  const auto active = application.lock();
  if (!active)
  {
    setEnabled(false);
    return;
  }
  const auto& state = active->session();
  const auto found = state.sources.find(state.active_source_id);
  if (found == state.sources.end())
  {
    setEnabled(false);
    return;
  }
  source = found->second.name;
  std::string error;
  const auto settings = active->vector_plot(source,error);
  setEnabled(settings.has_value());
  if (!settings)
  {
    return;
  }
  int selected = -1;
  const auto groups = active->field_groups(state.active_source_id,std::nullopt);
  for (const auto& group : groups)
  {
    Postprocessor::ResultFieldSelector selector;
    if (!Postprocessor::vector_field_choice(groups,group,selector))
    {
      continue;
    }
    if (settings->field && (settings->field->group_key == group.key ||
        (settings->field->group_key.empty() && settings->field->array_name == group.field.array_name)))
    {
      selected = fields->count();
    }
    selectors.push_back(selector);
    fields->addItem(QString::fromStdString(group.field.name+" / "+group.field.source+" / "+
      Postprocessor::field_association_name(group.field.association)+" / "+group.field.set_name));
  }
  if (!settings->field && !selectors.empty())
  {
    selected = 0;
  }
  fields->setCurrentIndex(selected);
  enabled->setChecked(settings->enabled);
  enabled->setEnabled(settings->field.has_value() || !selectors.empty());
  length->setCurrentIndex(!settings->proportional);
  samples->setCurrentIndex(!settings->surface_only);
  color->setCurrentIndex(!settings->magnitude_color);
  scale->setValue(settings->scale);
  maximum->setValue(static_cast<int>(settings->maximum));
  stride->setValue(static_cast<int>(settings->stride));
}
