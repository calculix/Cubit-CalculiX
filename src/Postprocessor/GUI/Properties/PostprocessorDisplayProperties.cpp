#include "PostprocessorDisplayProperties.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDoubleValidator>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QSpinBox>
#include <QVariant>
#include <QVBoxLayout>
#include <algorithm>
#include <cmath>
#include <exception>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <utility>

namespace
{
  std::string number(double value)
  {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return stream.str();
  }

  void set_color(QPushButton* button, const std::array<double,3>& rgb)
  {
    const auto color = QColor::fromRgbF(rgb[0],rgb[1],rgb[2]);
    button->setProperty("rgb",QVariantList{rgb[0],rgb[1],rgb[2]});
    button->setText(color.name(QColor::HexRgb).toUpper());
    button->setStyleSheet(QStringLiteral("background-color: %1; color: %2;").arg(color.name(),color.lightnessF()>0.5 ? "black":"white"));
  }

  std::array<double,3> color_value(const QPushButton* button)
  {
    const auto values = button->property("rgb").toList();
    return {values.value(0).toDouble(),values.value(1).toDouble(),values.value(2).toDouble()};
  }

  bool finite_value(const QLineEdit* input, double& value)
  {
    bool accepted = false;
    value = QLocale::c().toDouble(input->text(),&accepted);
    return accepted && input->hasAcceptableInput() && std::isfinite(value);
  }
}

PostprocessorDisplayProperties::PostprocessorDisplayProperties(
  std::weak_ptr<PostprocessorApplicationInterface> active, Submit dispatcher, QWidget* parent)
  : QWidget(parent), application(std::move(active)), submit(std::move(dispatcher))
{
  setObjectName("Postprocessor.DisplayProperties");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  auto* information = new QGroupBox(tr("Display"),this);
  auto* informationForm = new QFormLayout(information);
  view = new QLabel(information);
  field = new QLabel(information);
  component = new QLabel(information);
  field->setObjectName("Postprocessor.Display.Field");
  for (auto* label : {view,field,component})
  {
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
  }
  informationForm->addRow(tr("View"),view);
  informationForm->addRow(tr("Field"),field);
  informationForm->addRow(tr("Component"),component);
  layout->addWidget(information);

  auto* colors = new QGroupBox(tr("Colors"),this);
  auto* colorsForm = new QFormLayout(colors);
  palette = new QComboBox(colors);
  palette->setObjectName("Postprocessor.Display.Palette");
  using namespace Postprocessor;
  palette->addItem(tr("Viridis"),static_cast<int>(ColorPalette::Viridis));
  palette->addItem(tr("Blue - White - Red"),static_cast<int>(ColorPalette::BlueWhiteRed));
  palette->addItem(tr("Turbo"),static_cast<int>(ColorPalette::Turbo));
  palette->addItem(tr("Classic rainbow"),static_cast<int>(ColorPalette::Rainbow));
  palette->addItem(tr("Grayscale"),static_cast<int>(ColorPalette::Grayscale));
  reversed = new QCheckBox(tr("Reverse colors"),colors);
  reversed->setObjectName("Postprocessor.Display.Reverse");
  contours = new QComboBox(colors);
  contours->setObjectName("Postprocessor.Display.Contours");
  contours->addItem(tr("Continuous"),false);
  contours->addItem(tr("Banded"),true);
  bands = new QSpinBox(colors);
  bands->setObjectName("Postprocessor.Display.Bands");
  bands->setRange(2,static_cast<int>(ColoringDefaults::maximum_bands));
  colorsForm->addRow(tr("Palette"),palette);
  colorsForm->addRow(reversed);
  colorsForm->addRow(tr("Contours"),contours);
  colorsForm->addRow(tr("Bands"),bands);
  layout->addWidget(colors);

  auto numeric = [this](const char* name, QWidget* owner)
  {
    auto* input = new QLineEdit(owner);
    input->setObjectName(QString::fromLatin1(name));
    auto* validator = new QDoubleValidator(input);
    validator->setNotation(QDoubleValidator::ScientificNotation);
    auto locale = QLocale::c();
    locale.setNumberOptions(QLocale::RejectGroupSeparator);
    validator->setLocale(locale);
    input->setValidator(validator);
    input->setToolTip(tr("Finite result value; scientific notation is accepted. Uses the result's units."));
    connect(input,&QLineEdit::textChanged,this,[this]()
    {
      changed();
    });
    return input;
  };
  auto* rangeGroup = new QGroupBox(tr("Range"),this);
  auto* rangeForm = new QFormLayout(rangeGroup);
  range = new QComboBox(rangeGroup);
  range->setObjectName("Postprocessor.Display.Range");
  range->addItem(tr("Current frame"),static_cast<int>(ColorRangeMode::Automatic));
  range->addItem(tr("All frames"),static_cast<int>(ColorRangeMode::AllFrames));
  range->addItem(tr("Fixed"),static_cast<int>(ColorRangeMode::Fixed));
  range->addItem(tr("Symmetric"),static_cast<int>(ColorRangeMode::Symmetric));
  rangeMinimum = numeric("Postprocessor.Display.RangeMinimum",rangeGroup);
  rangeMaximum = numeric("Postprocessor.Display.RangeMaximum",rangeGroup);
  rangeMinimum->setToolTip(tr("Fixed range lower bound. Current-frame/All-frames/Symmetric ranges are computed from valid result data."));
  rangeMaximum->setToolTip(tr("Fixed range upper bound. Current-frame/All-frames/Symmetric ranges are computed from valid result data."));
  rangeForm->addRow(tr("Mode"),range);
  rangeForm->addRow(tr("Fixed minimum"),rangeMinimum);
  rangeForm->addRow(tr("Fixed maximum"),rangeMaximum);
  layout->addWidget(rangeGroup);

  auto* limitGroup = new QGroupBox(tr("Limits"),this);
  auto* limitForm = new QFormLayout(limitGroup);
  minimumEnabled = new QCheckBox(tr("Minimum"),limitGroup);
  maximumEnabled = new QCheckBox(tr("Maximum"),limitGroup);
  minimumEnabled->setObjectName("Postprocessor.Display.MinimumEnabled");
  maximumEnabled->setObjectName("Postprocessor.Display.MaximumEnabled");
  minimumValue = numeric("Postprocessor.Display.MinimumValue",limitGroup);
  maximumValue = numeric("Postprocessor.Display.MaximumValue",limitGroup);
  minimumColor = new QPushButton(limitGroup);
  maximumColor = new QPushButton(limitGroup);
  minimumColor->setObjectName("Postprocessor.Display.MinimumColor");
  maximumColor->setObjectName("Postprocessor.Display.MaximumColor");
  minimumColor->setToolTip(tr("Color for valid values strictly below the minimum limit"));
  maximumColor->setToolTip(tr("Color for valid values strictly above the maximum limit"));
  auto* minimumRow = new QHBoxLayout;
  minimumRow->addWidget(minimumValue,1);
  minimumRow->addWidget(minimumColor);
  auto* maximumRow = new QHBoxLayout;
  maximumRow->addWidget(maximumValue,1);
  maximumRow->addWidget(maximumColor);
  limitForm->addRow(minimumEnabled,minimumRow);
  limitForm->addRow(maximumEnabled,maximumRow);
  layout->addWidget(limitGroup);
  legend = new QCheckBox(tr("Show legend"),this);
  legend->setObjectName("Postprocessor.Display.Legend");
  layout->addWidget(legend);
  errorMessage = new QLabel(this);
  errorMessage->setObjectName("Postprocessor.Display.Error");
  errorMessage->setTextFormat(Qt::PlainText);
  errorMessage->setWordWrap(true);
  layout->addWidget(errorMessage);
  applyButton = new QPushButton(tr("Apply"),this);
  applyButton->setObjectName("Postprocessor.Display.Apply");
  applyButton->setToolTip(tr("Validate and apply display edits through journaled commands"));
  layout->addWidget(applyButton,0,Qt::AlignRight);

  for (auto* combo : {palette,contours,range})
  {
    connect(combo,&QComboBox::currentIndexChanged,this,[this]()
    {
      changed();
    });
  }
  for (auto* check : {reversed,minimumEnabled,maximumEnabled,legend})
  {
    connect(check,&QCheckBox::toggled,this,[this]()
    {
      changed();
    });
  }
  connect(bands,&QSpinBox::valueChanged,this,[this]()
  {
    changed();
  });
  for (auto* button : {minimumColor,maximumColor})
  {
    connect(button,&QPushButton::clicked,this,[this,button]()
    {
      choose_color(button);
    });
  }
  connect(applyButton,&QPushButton::clicked,this,[this]()
  {
    std::string error;
    if (!apply(error))
    {
      errorMessage->setText(QString::fromStdString(error));
    }
  });
  refresh();
}

bool PostprocessorDisplayProperties::current_context(Context& context) const
{
  const auto active = application.lock();
  if (!active)
  {
    return false;
  }
  const auto& session = active->session();
  const auto source = session.sources.find(session.active_source_id);
  const auto selectedView = session.views.find(session.active_view_id);
  if (source == session.sources.end() || selectedView == session.views.end())
  {
    return false;
  }
  for (const auto& entry : session.representations)
  {
    const auto& display = entry.second;
    if (display.input.owner == Postprocessor::OutputOwner::Source && display.input.object_id == source->first &&
        display.input.port == 0 && display.view_id == selectedView->first)
    {
      context = {source->first,source->second.name,selectedView->first,selectedView->second.name,
        entry.first,display.increment_index,active->result_data(source->first),display.coloring};
      return true;
    }
  }
  return false;
}

bool PostprocessorDisplayProperties::same_context(const Context& context) const
{
  return available && context.source_id == baseline.source_id && context.source_name == baseline.source_name && context.view_id == baseline.view_id &&
    context.representation_id == baseline.representation_id && context.data == baseline.data;
}

void PostprocessorDisplayProperties::refresh()
{
  if (applying)
  {
    return;
  }
  Context current;
  if (!current_context(current))
  {
    available = false;
    baseline = {};
    load(Postprocessor::ResultColoring{});
    view->clear();
    field->clear();
    component->clear();
    errorMessage->clear();
    setEnabled(false);
    return;
  }
  const bool reload = !same_context(current) || !Postprocessor::same_coloring(baseline.coloring,current.coloring);
  available = true;
  baseline = current;
  setEnabled(true);
  if (reload)
  {
    load(current.coloring);
    errorMessage->clear();
  }
  view->setText(QString::fromUtf8(current.view_name.c_str()));
  field->setText(tr("Solid color"));
  component->clear();
  if (current.coloring.field)
  {
    const auto& selection = *current.coloring.field;
    const auto* metadata = current.data && current.increment < current.data->increments.size() ?
      Postprocessor::find_field(current.data->increments[current.increment],selection) : nullptr;
    std::string label = metadata ? metadata->name : selection.array_name;
    if (!metadata)
    {
      const auto active = application.lock();
      if (active)
      {
        for (const auto& group : active->field_groups(current.source_id,current.increment))
        {
          if (group.field.array_name == selection.array_name && group.field.association == selection.association &&
              (selection.group_key.empty() || group.key == selection.group_key) &&
              (!selection.dataset_id || std::find(group.datasets.begin(),group.datasets.end(),*selection.dataset_id) != group.datasets.end()))
          {
            label = group.field.name;
            break;
          }
        }
      }
    }
    field->setText(QString::fromUtf8(label.c_str())+(metadata ? QString() : tr(" (unavailable)")));
    component->setText(selection.magnitude ? tr("Magnitude") : QString::fromUtf8(selection.component.c_str()));
  }
  update_enabled();
}

void PostprocessorDisplayProperties::load(const Postprocessor::ResultColoring& coloring)
{
  const QScopedValueRollback<bool> guard(loading,true);
  palette->setCurrentIndex(palette->findData(static_cast<int>(coloring.palette)));
  reversed->setChecked(coloring.reversed);
  contours->setCurrentIndex(contours->findData(coloring.bands.has_value()));
  bands->setValue(static_cast<int>(coloring.bands.value_or(Postprocessor::ColoringDefaults::default_bands)));
  range->setCurrentIndex(range->findData(static_cast<int>(coloring.range_mode)));
  rangeMinimum->setText(QString::fromStdString(number(coloring.fixed_range[0])));
  rangeMaximum->setText(QString::fromStdString(number(coloring.fixed_range[1])));
  minimumEnabled->setChecked(coloring.minimum.enabled);
  maximumEnabled->setChecked(coloring.maximum.enabled);
  minimumValue->setText(QString::fromStdString(number(coloring.minimum.value)));
  maximumValue->setText(QString::fromStdString(number(coloring.maximum.value)));
  set_color(minimumColor,coloring.minimum.color);
  set_color(maximumColor,coloring.maximum.color);
  legend->setChecked(coloring.legend_visible);
  dirty = false;
  update_enabled();
}

void PostprocessorDisplayProperties::changed()
{
  if (loading)
  {
    return;
  }
  dirty = true;
  errorMessage->clear();
  update_enabled();
}

void PostprocessorDisplayProperties::update_enabled()
{
  const bool fixed = range->currentData().toInt() == static_cast<int>(Postprocessor::ColorRangeMode::Fixed);
  rangeMinimum->setEnabled(available && fixed);
  rangeMaximum->setEnabled(available && fixed);
  bands->setEnabled(available && contours->currentData().toBool());
  minimumValue->setEnabled(available && minimumEnabled->isChecked());
  minimumColor->setEnabled(available && minimumEnabled->isChecked());
  maximumValue->setEnabled(available && maximumEnabled->isChecked());
  maximumColor->setEnabled(available && maximumEnabled->isChecked());
  applyButton->setEnabled(available && dirty && !applying);
}

void PostprocessorDisplayProperties::choose_color(QPushButton* button)
{
  if (!available)
  {
    return;
  }
  const auto owner = baseline;
  const auto rgb = color_value(button);
  const auto color = QColorDialog::getColor(QColor::fromRgbF(rgb[0],rgb[1],rgb[2]),this,tr("Result limit color"));
  Context current;
  if (color.isValid() && current_context(current) && same_context(current) &&
      owner.source_id == baseline.source_id && owner.source_name == baseline.source_name &&
      owner.view_id == baseline.view_id && owner.representation_id == baseline.representation_id && owner.data == baseline.data &&
      Postprocessor::same_coloring(owner.coloring,baseline.coloring) && Postprocessor::same_coloring(current.coloring,baseline.coloring))
  {
    set_color(button,{color.redF(),color.greenF(),color.blueF()});
    changed();
  }
}

bool PostprocessorDisplayProperties::draft(Postprocessor::ResultColoring& result,std::string& error) const
{
  result = baseline.coloring;
  result.palette = static_cast<Postprocessor::ColorPalette>(palette->currentData().toInt());
  result.reversed = reversed->isChecked();
  result.bands = contours->currentData().toBool() ? std::optional<unsigned int>(static_cast<unsigned int>(bands->value())) : std::nullopt;
  result.range_mode = static_cast<Postprocessor::ColorRangeMode>(range->currentData().toInt());
  if (result.range_mode == Postprocessor::ColorRangeMode::Fixed &&
      (!finite_value(rangeMinimum,result.fixed_range[0]) || !finite_value(rangeMaximum,result.fixed_range[1])))
  {
    error = "Fixed range requires two finite numeric values.";
    return false;
  }
  result.minimum.enabled = minimumEnabled->isChecked();
  result.maximum.enabled = maximumEnabled->isChecked();
  auto limitValue = [&error](const QLineEdit* input,Postprocessor::ColorLimit& limit)
  {
    double value = limit.value;
    if (finite_value(input,value))
    {
      limit.value = value;
    }
    else if (limit.enabled)
    {
      error = "Enabled result limits require finite numeric thresholds.";
      return false;
    }
    return true;
  };
  if (!limitValue(minimumValue,result.minimum) || !limitValue(maximumValue,result.maximum))
  {
    return false;
  }
  result.minimum.color = color_value(minimumColor);
  result.maximum.color = color_value(maximumColor);
  result.legend_visible = legend->isChecked();
  return Postprocessor::valid_coloring(result,error);
}

bool PostprocessorDisplayProperties::commands(const Postprocessor::ResultColoring& target,
  std::vector<Command>& plan,std::string& error) const
{
  using namespace Postprocessor;
  const auto name = QString::fromUtf8(baseline.source_name.c_str());
  const QChar delimiter = name.contains('"') ? QChar(0x27) : QChar('"');
  if (name.isEmpty() || name.contains(delimiter))
  {
    error = "The source name cannot be represented as one quoted command argument.";
    return false;
  }
  for (QChar c : name)
  {
    if (c.unicode() < 32 || c.unicode() == 127)
    {
      error = "The source name contains a control character.";
      return false;
    }
  }
  const auto source = (delimiter+name+delimiter).toUtf8().toStdString();
  auto staged = baseline.coloring;
  auto append = [&](const std::string& operation,auto edit)
  {
    edit(staged);
    if (!valid_coloring(staged,error))
    {
      return false;
    }
    plan.push_back({"post "+operation+" source "+source,staged});
    return true;
  };
  if (staged.palette != target.palette)
  {
    const char* value = nullptr;
    switch (target.palette)
    {
      case ColorPalette::Viridis: value = "viridis"; break;
      case ColorPalette::BlueWhiteRed: value = "bluewhitered"; break;
      case ColorPalette::Turbo: value = "turbo"; break;
      case ColorPalette::Rainbow: value = "rainbow"; break;
      case ColorPalette::Grayscale: value = "grayscale"; break;
    }
    if (value == nullptr || !append(std::string("color palette ")+value,[&](ResultColoring& c)
      {
        c.palette = target.palette;
      }))
    {
      return false;
    }
  }
  if (staged.reversed != target.reversed && !append(target.reversed ? "color reverse on" : "color reverse off",[&](ResultColoring& c)
    {
      c.reversed = target.reversed;
    }))
  {
    return false;
  }
  if (staged.bands != target.bands && !append(target.bands ? "color bands "+std::to_string(*target.bands) : "color bands continuous",[&](ResultColoring& c)
    {
      c.bands = target.bands;
    }))
  {
    return false;
  }
  if (staged.range_mode != target.range_mode ||
      (target.range_mode == ColorRangeMode::Fixed && staged.fixed_range != target.fixed_range))
  {
    std::string operation = "color range ";
    switch (target.range_mode)
    {
      case ColorRangeMode::Automatic: operation += "auto"; break;
      case ColorRangeMode::AllFrames: operation += "allframes"; break;
      case ColorRangeMode::Symmetric: operation += "symmetric"; break;
      case ColorRangeMode::Fixed: operation += number(target.fixed_range[0])+" "+number(target.fixed_range[1]); break;
    }
    if (!append(operation,[&](ResultColoring& c)
      {
        c.range_mode = target.range_mode;
        c.fixed_range = target.fixed_range;
      }))
    {
      return false;
    }
  }
  const bool valuesChanged = staged.minimum.value != target.minimum.value || staged.minimum.color != target.minimum.color ||
    staged.maximum.value != target.maximum.value || staged.maximum.color != target.maximum.color;
  // The existing value/color command enables that limit. Temporarily disable
  // both endpoints when configuring values, so every intermediate state is valid,
  // including retained but disabled thresholds that cross an enabled endpoint.
  for (bool minimum : {true,false})
  {
    const auto& old = minimum ? staged.minimum : staged.maximum;
    const auto& desired = minimum ? target.minimum : target.maximum;
    if (old.enabled && (valuesChanged || !desired.enabled))
    {
      if (!append(minimum ? "color limit minimum off" : "color limit maximum off",[minimum](ResultColoring& c)
        {
          (minimum ? c.minimum : c.maximum).enabled = false;
        }))
      {
        return false;
      }
    }
  }
  for (bool minimum : {true,false})
  {
    const auto old = minimum ? staged.minimum : staged.maximum;
    const auto desired = minimum ? target.minimum : target.maximum;
    if (old.value != desired.value || old.color != desired.color)
    {
      const auto operation = std::string("color limit ")+(minimum ? "minimum " : "maximum ")+number(desired.value)+
        " color "+number(desired.color[0])+" "+number(desired.color[1])+" "+number(desired.color[2]);
      if (!append(operation,[minimum,desired](ResultColoring& c)
        {
          auto& limit = minimum ? c.minimum : c.maximum;
          limit.value = desired.value;
          limit.color = desired.color;
          limit.enabled = true;
        }) || !append(minimum ? "color limit minimum off" : "color limit maximum off",[minimum](ResultColoring& c)
        {
          (minimum ? c.minimum : c.maximum).enabled = false;
        }))
      {
        return false;
      }
    }
  }
  for (bool minimum : {true,false})
  {
    const auto& old = minimum ? staged.minimum : staged.maximum;
    const auto& desired = minimum ? target.minimum : target.maximum;
    if (!old.enabled && desired.enabled && !append(minimum ? "color limit minimum on" : "color limit maximum on",[minimum](ResultColoring& c)
      {
        (minimum ? c.minimum : c.maximum).enabled = true;
      }))
    {
      return false;
    }
  }
  if (staged.legend_visible != target.legend_visible && !append(target.legend_visible ? "legend on" : "legend off",[&](ResultColoring& c)
    {
      c.legend_visible = target.legend_visible;
    }))
  {
    return false;
  }
  if (!same_coloring(staged,target))
  {
    error = "The display command plan does not reproduce the edited settings.";
    return false;
  }
  return true;
}

void PostprocessorDisplayProperties::recover(const std::string& error)
{
  available = false;
  refresh();
  errorMessage->setText(QString::fromStdString(error));
}

bool PostprocessorDisplayProperties::apply(std::string& error)
{
  error.clear();
  if (applying)
  {
    error = "Display settings are already being applied.";
    return false;
  }
  try
  {
    Context current;
    if (!current_context(current) || !same_context(current) ||
        !Postprocessor::same_coloring(current.coloring,baseline.coloring))
    {
      error = "The display context changed. Review the current settings before applying.";
      recover(error);
      return false;
    }
    Postprocessor::ResultColoring target;
    std::vector<Command> plan;
    if (!draft(target,error) || !commands(target,plan,error))
    {
      errorMessage->setText(QString::fromStdString(error));
      return false;
    }
    bool accepted = true;
    {
      const QScopedValueRollback<bool> guard(applying,true);
      update_enabled();
      auto expected = baseline.coloring;
      for (const auto& step : plan)
      {
        if (!current_context(current) || !same_context(current) || !Postprocessor::same_coloring(current.coloring,expected))
        {
          error = "Display settings changed during Apply. Remaining commands were cancelled.";
          accepted = false;
          break;
        }
        if (!submit || !submit(step.text,error))
        {
          if (error.empty())
          {
            error = "The display command could not be submitted.";
          }
          accepted = false;
          break;
        }
        if (!current_context(current) || !same_context(current) || !Postprocessor::same_coloring(current.coloring,step.expected))
        {
          error = "The display command did not publish the requested settings. The panel shows the committed state.";
          accepted = false;
          break;
        }
        expected = step.expected;
      }
    }
    if (!accepted)
    {
      recover(error);
      return false;
    }
    available = false;
    refresh();
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot apply display settings: ")+exception.what();
  }
  catch (...)
  {
    error = "Cannot apply display settings: unknown failure.";
  }
  // Recovery may itself require allocations. Preserve the original error if it
  // cannot refresh during exceptional resource exhaustion.
  try
  {
    recover(error);
  }
  catch (...)
  {
  }
  return false;
}
