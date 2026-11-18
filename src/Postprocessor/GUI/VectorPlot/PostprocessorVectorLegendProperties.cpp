#include "PostprocessorVectorLegendProperties.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/results/VectorCommandText.hpp"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QSpinBox>
#include <QVariant>
#include <cmath>
#include <utility>
namespace
{
  QString number(double value)
  {
    return QString::number(value,'g',17);
  }
  void set_color(QPushButton* button,const std::array<double,3>& rgb)
  {
    const auto color = QColor::fromRgbF(rgb[0],rgb[1],rgb[2]);
    button->setProperty("rgb",QVariantList{rgb[0],rgb[1],rgb[2]});
    button->setText(color.name().toUpper());
    QString ink = "white";
    if (color.lightnessF() > .5)
    {
      ink = "black";
    }
    button->setStyleSheet(QString("background-color:%1;color:%2;").arg(color.name(),ink));
  }
}
PostprocessorVectorLegendProperties::PostprocessorVectorLegendProperties(std::weak_ptr<PostprocessorApplicationInterface> app,
  Submit dispatcher,QWidget* parent) : QWidget(parent),application(std::move(app)),submit(std::move(dispatcher))
{
  setObjectName("Postprocessor.VectorLegendProperties");
  auto* form = new QFormLayout(this);
  information = new QLabel(this);
  information->setTextFormat(Qt::PlainText);
  information->setWordWrap(true);
  form->addRow(information);
  visible = new QCheckBox(tr("Show vector legend"),this);
  visible->setObjectName("Postprocessor.VectorLegend.Visible");
  form->addRow(visible);
  palette = new QComboBox(this);
  palette->setObjectName("Postprocessor.VectorLegend.Palette");
  for (const auto value : {Postprocessor::ColorPalette::Viridis,Postprocessor::ColorPalette::BlueWhiteRed,
      Postprocessor::ColorPalette::Turbo,Postprocessor::ColorPalette::Rainbow,Postprocessor::ColorPalette::Grayscale})
  {
    palette->addItem(QString::fromLatin1(Postprocessor::color_palette_name(value)),static_cast<int>(value));
  }
  form->addRow(tr("Palette"),palette);
  reversed = new QCheckBox(tr("Reverse colors"),this);
  form->addRow(reversed);
  contours = new QComboBox(this);
  contours->addItems({tr("Continuous"),tr("Banded")});
  form->addRow(tr("Contours"),contours);
  bands = new QSpinBox(this);
  bands->setRange(2,static_cast<int>(Postprocessor::ColoringDefaults::maximum_bands));
  bands->setKeyboardTracking(false);
  form->addRow(tr("Bands"),bands);
  auto* colorPanel = new QWidget(this);
  colors = new QGridLayout(colorPanel);
  colors->setContentsMargins(0,0,0,0);
  form->addRow(tr("Palette colors"),colorPanel);
  auto* resetColors = new QPushButton(tr("Reset palette colors"),this);
  form->addRow(resetColors);
  range = new QComboBox(this);
  range->addItems({tr("Current frame"),tr("Fixed"),tr("Symmetric"),tr("All frames")});
  form->addRow(tr("Range"),range);
  const auto numeric = [this]()
  {
    auto* input = new QLineEdit(this);
    auto* validator = new QDoubleValidator(input);
    validator->setLocale(QLocale::c());
    input->setValidator(validator);
    return input;
  };
  low = numeric();
  high = numeric();
  form->addRow(tr("Fixed minimum"),low);
  form->addRow(tr("Fixed maximum"),high);
  minimumEnabled = new QCheckBox(tr("Minimum limit"),this);
  maximumEnabled = new QCheckBox(tr("Maximum limit"),this);
  minimumValue = numeric();
  maximumValue = numeric();
  minimumColor = new QPushButton(this);
  maximumColor = new QPushButton(this);
  form->addRow(minimumEnabled,minimumValue);
  form->addRow(tr("Below minimum color"),minimumColor);
  form->addRow(maximumEnabled,maximumValue);
  form->addRow(tr("Above maximum color"),maximumColor);
  auto* automatic = new QPushButton(tr("Automatic position"),this);
  form->addRow(automatic);
  auto* defaults = new QPushButton(tr("Defaults"),this);
  defaults->setObjectName("Postprocessor.VectorLegend.Defaults");
  form->addRow(defaults);
  errorLabel = new QLabel(this);
  errorLabel->setWordWrap(true);
  errorLabel->setTextFormat(Qt::PlainText);
  form->addRow(errorLabel);
  connect(visible,&QCheckBox::clicked,this,[this](bool on)
  {
    QString operation = "off";
    if (on)
    {
      operation = "on";
    }
    send(operation);
  });
  connect(reversed,&QCheckBox::clicked,this,[this](bool on)
  {
    QString operation = "reverse off";
    if (on)
    {
      operation = "reverse on";
    }
    send(operation);
  });
  connect(palette,&QComboBox::activated,this,[this](int)
  {
    send("palette "+QString::fromLatin1(Postprocessor::color_palette_name(static_cast<Postprocessor::ColorPalette>(palette->currentData().toInt()))));
  });
  connect(contours,&QComboBox::activated,this,[this](int index)
  {
    QString operation = "bands continuous";
    if (index == 1)
    {
      operation = "bands "+QString::number(bands->value());
    }
    send(operation);
  });
  connect(bands,&QSpinBox::valueChanged,this,[this](int value)
  {
    send("bands "+QString::number(value));
  });
  connect(resetColors,&QPushButton::clicked,this,[this]()
  {
    send("palette reset");
  });
  connect(range,&QComboBox::activated,this,[this](int)
  {
    range_changed();
  });
  for (auto* input : {low,high})
  {
    connect(input,&QLineEdit::editingFinished,this,[this]()
    {
      range_changed();
    });
  }
  for (auto* input : {minimumValue,maximumValue})
  {
    connect(input,&QLineEdit::editingFinished,this,[this]()
    {
      limits_changed();
    });
  }
  for (auto* check : {minimumEnabled,maximumEnabled})
  {
    connect(check,&QCheckBox::clicked,this,[this]()
    {
      limits_changed();
    });
  }
  for (auto* button : {minimumColor,maximumColor})
  {
    connect(button,&QPushButton::clicked,this,[this,button]()
    {
      const auto owner = baseline;
      const auto sourceName = source;
      const auto view = viewId;
      const auto color = QColorDialog::getColor(QColor(button->text()),this,tr("Vector legend limit color"));
      const auto app = application.lock();
      std::string error;
      if (!app || !owner || !color.isValid() || app->session().active_view_id != view || source != sourceName)
      {
        return;
      }
      const auto current = app->vector_plot(source,error);
      if (!current || !Postprocessor::same_coloring(owner->legend,current->legend))
      {
        refresh();
        return;
      }
      set_color(button,{color.redF(),color.greenF(),color.blueF()});
      limits_changed();
    });
  }
  connect(automatic,&QPushButton::clicked,this,[this]()
  {
    send("auto");
  });
  connect(defaults,&QPushButton::clicked,this,[this]()
  {
    send("defaults");
  });
  refresh();
}
void PostprocessorVectorLegendProperties::send(const QString& operation)
{
  if (loading)
  {
    return;
  }
  const auto app = application.lock();
  std::string error;
  if (!app || app->session().active_view_id != viewId)
  {
    refresh();
    return;
  }
  const auto command = Postprocessor::vector_command_text("legend "+operation,source);
  if (command.isEmpty() || !submit || !submit(command.toStdString(),error))
  {
    errorLabel->setText(QString::fromStdString(error));
    return;
  }
  errorLabel->clear();
  refresh();
}
void PostprocessorVectorLegendProperties::range_changed()
{
  low->setEnabled(range->currentIndex() == 1);
  high->setEnabled(range->currentIndex() == 1);
  const char* modes[] = {"auto","fixed","symmetric","allframes"};
  if (range->currentIndex() != 1)
  {
    send("range "+QString::fromLatin1(modes[range->currentIndex()]));
    return;
  }
  bool a = false,b = false;
  const double x = QLocale::c().toDouble(low->text(),&a),y = QLocale::c().toDouble(high->text(),&b);
  if (!a || !b || !std::isfinite(x) || !std::isfinite(y) || x >= y)
  {
    errorLabel->setText(tr("Enter finite, ordered fixed bounds."));
    return;
  }
  send("range "+number(x)+" "+number(y));
}
void PostprocessorVectorLegendProperties::limits_changed()
{
  QString operation = "limits";
  for (bool minimum : {true,false})
  {
    auto* enabled = maximumEnabled;
    auto* input = maximumValue;
    auto* color = maximumColor;
    QString name = "maximum";
    if (minimum)
    {
      enabled = minimumEnabled;
      input = minimumValue;
      color = minimumColor;
      name = "minimum";
    }
    bool valid = false;
    const double value = QLocale::c().toDouble(input->text(),&valid);
    if (!valid || !std::isfinite(value))
    {
      errorLabel->setText(tr("Enter finite limit values."));
      return;
    }
    QString state = "off";
    if (enabled->isChecked())
    {
      state = "on";
    }
    operation += " "+name+" \""+state+"\" "+number(value)+" color";
    for (const auto& channel : color->property("rgb").toList())
    {
      operation += " "+number(channel.toDouble());
    }
  }
  send(operation);
}
void PostprocessorVectorLegendProperties::rebuild_colors(const Postprocessor::ResultColoring& coloring)
{
  const auto samples = Postprocessor::palette_endpoints(coloring);
  if (samples == swatches)
  {
    return;
  }
  swatches = samples;
  while (auto* item = colors->takeAt(0))
  {
    item->widget()->hide();
    item->widget()->deleteLater();
    delete item;
  }
  for (std::size_t i = 0; i < samples.size(); ++i)
  {
    auto* button = new QPushButton(this);
    const auto rgb = samples[i];
    const auto color = QColor::fromRgbF(rgb[0],rgb[1],rgb[2]);
    const bool high = i != 0;
    QString label = tr("Low color");
    QString endpoint = "low";
    if (high)
    {
      label = tr("High color");
      endpoint = "high";
    }
    QString textColor = "#000000";
    if (.2126*rgb[0]+.7152*rgb[1]+.0722*rgb[2] < .5)
    {
      textColor = "#FFFFFF";
    }
    button->setText(label);
    button->setStyleSheet("background-color:"+color.name()+";color:"+textColor+";");
    button->setToolTip(color.name());
    colors->addWidget(button,0,static_cast<int>(i));
    connect(button,&QPushButton::clicked,this,[this,endpoint,color]()
    {
      const auto owner = baseline;
      const auto sourceName = source;
      const auto view = viewId;
      const auto selected = QColorDialog::getColor(color,this,tr("Vector palette color"));
      const auto app = application.lock();
      std::string error;
      if (!selected.isValid() || !app || !owner || source != sourceName || app->session().active_view_id != view)
      {
        return;
      }
      const auto current = app->vector_plot(source,error);
      if (!current || !Postprocessor::same_coloring(owner->legend,current->legend))
      {
        refresh();
        return;
      }
      send("palette "+endpoint+" color "+number(selected.redF())+" "+number(selected.greenF())+" "+number(selected.blueF()));
    });
  }
}
void PostprocessorVectorLegendProperties::refresh()
{
  const QScopedValueRollback<bool> guard(loading,true);
  const auto app = application.lock();
  if (!app)
  {
    setEnabled(false);
    return;
  }
  const auto& state = app->session();
  const auto found = state.sources.find(state.active_source_id);
  if (found == state.sources.end())
  {
    setEnabled(false);
    return;
  }
  std::string error;
  const auto value = app->vector_plot(found->second.name,error);
  setEnabled(value.has_value());
  if (!value)
  {
    return;
  }
  const bool same = source == found->second.name && viewId == state.active_view_id && baseline &&
    Postprocessor::same_coloring(baseline->legend,value->legend) && Postprocessor::same_field(baseline->field,value->field);
  source = found->second.name;
  viewId = state.active_view_id;
  baseline = value;
  if (same)
  {
    return;
  }
  const auto& c = value->legend;
  QString unit = tr("unknown unit");
  if (c.bounds_unit && !c.bounds_unit->unit.empty())
  {
    unit = QString::fromStdString(c.bounds_unit->unit);
  }
  QString field = tr("No vector field selected");
  if (value->field)
  {
    for (const auto& group : app->field_groups(state.active_source_id,std::nullopt))
    {
      if (group.key == value->field->group_key)
      {
        field = QString::fromUtf8(group.field.name.c_str());
        break;
      }
    }
  }
  information->setText(QString::fromUtf8(source.c_str())+"\n"+field+" / "+tr("Magnitude")+" ("+unit+")");
  visible->setChecked(c.legend_visible);
  palette->setCurrentIndex(palette->findData(static_cast<int>(c.palette)));
  reversed->setChecked(c.reversed);
  contours->setCurrentIndex(static_cast<int>(c.bands.has_value()));
  bands->setValue(static_cast<int>(c.bands.value_or(Postprocessor::ColoringDefaults::default_bands)));
  bands->setEnabled(c.bands.has_value());
  range->setCurrentIndex(static_cast<int>(c.range_mode));
  low->setText(number(c.fixed_range[0]));
  high->setText(number(c.fixed_range[1]));
  low->setEnabled(c.range_mode == Postprocessor::ColorRangeMode::Fixed);
  high->setEnabled(c.range_mode == Postprocessor::ColorRangeMode::Fixed);
  low->setToolTip(tr("Value in %1").arg(unit));
  high->setToolTip(tr("Value in %1").arg(unit));
  minimumEnabled->setChecked(c.minimum.enabled);
  maximumEnabled->setChecked(c.maximum.enabled);
  minimumValue->setText(number(c.minimum.value));
  maximumValue->setText(number(c.maximum.value));
  set_color(minimumColor,c.minimum.color);
  set_color(maximumColor,c.maximum.color);
  rebuild_colors(c);
}
