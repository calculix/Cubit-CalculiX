#include "PostprocessorDeformationControls.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleValidator>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QEvent>
#include <QHideEvent>
#include <QKeyEvent>
#include <QTimer>
#include <QSizePolicy>
#include <QStandardItemModel>
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
  std::string quote(const std::string& value)
  {
    const char delimiter = value.find('"') == std::string::npos ? '"' : '\'';
    if (value.empty() || value.find(delimiter) != std::string::npos ||
        std::any_of(value.begin(),value.end(),[](unsigned char c)
        {
          return c < 32 || c == 127;
        }))
    {
      return {};
    }
    return std::string(1,delimiter)+value+delimiter;
  }
  std::string number(double value)
  {
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return stream.str();
  }
}

PostprocessorDeformationControls::PostprocessorDeformationControls(
  std::weak_ptr<PostprocessorApplicationInterface> active, Submit dispatcher, Submit preview, QWidget* parent)
  : QWidget(parent), application(std::move(active)), submit(std::move(dispatcher)), silent(std::move(preview))
{
  setObjectName("Postprocessor.DeformationControls");
  setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(4);
  enabled = new QCheckBox(tr("Deform"),this);
  enabled->setObjectName("Postprocessor.Deformation.Enabled");
  enabled->setToolTip(tr("Immediately enable or disable deformation of the selected source in the active view."));
  layout->addWidget(enabled);
  fields = new QComboBox(this);
  fields->setObjectName("Postprocessor.Deformation.Field");
  fields->setMinimumContentsLength(8);
  fields->setMaximumWidth(180);
  fields->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  fields->setToolTip(tr("Immediately select Automatic displacement or an explicit vector for the active source and view."));
  layout->addWidget(fields);
  layout->addWidget(new QLabel(tr("Scale"),this));
  scale = new QLineEdit(this);
  scale->setObjectName("Postprocessor.Deformation.Scale");
  scale->setMaximumWidth(85);
  scale->setToolTip(tr("Finite nonnegative multiplier; units must match. Valid edits preview after a short pause. Enter or leaving the field journals the final value once."));
  auto* validator = new QDoubleValidator(scale);
  validator->setBottom(0);
  validator->setNotation(QDoubleValidator::ScientificNotation);
  auto locale = QLocale::c();
  locale.setNumberOptions(QLocale::RejectGroupSeparator);
  validator->setLocale(locale);
  scale->setValidator(validator);
  layout->addWidget(scale);
  overlay = new QCheckBox(tr("Undeformed"),this);
  overlay->setObjectName("Postprocessor.Deformation.Overlay");
  overlay->setToolTip(tr("Show the original mesh as neutral wireframe while deformation is enabled."));
  layout->addWidget(overlay);
  scaleTimer = new QTimer(this);
  scaleTimer->setObjectName("Postprocessor.Deformation.ScalePreviewTimer");
  scaleTimer->setSingleShot(true);
  scaleTimer->setInterval(Postprocessor::DeformationDefaults::scale_preview_delay_ms);
  scale->installEventFilter(this);
  errorLabel = new QLabel(this);
  errorLabel->setObjectName("Postprocessor.Deformation.Error");
  layout->addWidget(errorLabel);
  connect(enabled,&QCheckBox::toggled,this,[this](bool value)
  {
    change_flag(true,value);
  });
  connect(overlay,&QCheckBox::toggled,this,[this](bool value)
  {
    change_flag(false,value);
  });
  connect(fields,&QComboBox::activated,this,[this](int index)
  {
    select_field(index);
  });
  connect(scale,&QLineEdit::textEdited,this,[this](const QString&)
  {
    edit_scale();
  });
  connect(scale,&QLineEdit::editingFinished,this,[this]()
  {
    finish_scale();
  });
  connect(scaleTimer,&QTimer::timeout,this,[this]()
  {
    preview_scale();
  });
  refresh();
}

std::optional<PostprocessorDeformationControls::Context> PostprocessorDeformationControls::current() const
{
  const auto active = application.lock();
  if (!active)
  {
    return std::nullopt;
  }
  const auto& state = active->session();
  const auto source = state.sources.find(state.active_source_id);
  if (source == state.sources.end())
  {
    return std::nullopt;
  }
  for (const auto& entry : state.representations)
  {
    const auto& display = entry.second;
    if (display.input.owner == Postprocessor::OutputOwner::Source && display.input.port == 0 &&
        display.input.object_id == source->first && display.view_id == state.active_view_id)
    {
      const auto data = active->result_data(source->first);
      if (data && display.increment_index < data->increments.size())
      {
        return Context{source->first,source->second.name,display.view_id,entry.first,
          display.increment_index,data,display.deformation};
      }
    }
  }
  return std::nullopt;
}

bool PostprocessorDeformationControls::same_context(const Context& a, const Context& b) const
{
  return a.source == b.source && a.name == b.name && a.view == b.view &&
    a.representation == b.representation && a.data == b.data;
}

void PostprocessorDeformationControls::populate_fields(const Context& context,
  const Postprocessor::ResultDeformation& settings)
{
  fields->clear();
  fields->addItem(tr("Automatic displacement"),-1);
  const auto& selection = settings.field;
  const auto active = application.lock();
  catalogue = active ? active->field_groups(context.source,context.frame) : std::vector<Postprocessor::ResultFieldGroup>{};
  int selected = 0;
  for (std::size_t i = 0; i < catalogue.size(); ++i)
  {
    const auto& group = catalogue[i];
    if (!Postprocessor::deformation_vector(group.field))
    {
      continue;
    }
    auto label = QString::fromUtf8(group.field.name.c_str());
    if (std::count_if(catalogue.begin(),catalogue.end(),[&](const Postprocessor::ResultFieldGroup& other)
        {
          return other.field.name == group.field.name && Postprocessor::deformation_vector(other.field);
        }) > 1)
    {
      label += " ["+QString::fromUtf8(group.field.array_name.c_str())+" / "+
        QString::number(static_cast<qulonglong>(group.field.dataset_id))+"]";
    }
    if (!group.available)
    {
      label += tr(" (unavailable)");
    }
    fields->addItem(label,static_cast<int>(i));
    if (settings.field_mode == Postprocessor::DeformationFieldMode::Explicit &&
        selection && selection->group_key == group.key && (!selection->dataset_id ||
        std::find(group.datasets.begin(),group.datasets.end(),*selection->dataset_id) != group.datasets.end()))
    {
      selected = fields->count()-1;
    }
    if (auto* model = qobject_cast<QStandardItemModel*>(fields->model()))
    {
      model->item(fields->count()-1)->setEnabled(group.available);
    }
  }
  if (settings.field_mode == Postprocessor::DeformationFieldMode::Explicit && selected == 0)
  {
    fields->addItem(tr("Selected vector unavailable"),-2);
    selected = fields->count()-1;
  }
  fields->setCurrentIndex(selected);
}

void PostprocessorDeformationControls::refresh()
{
  if (dispatching)
  {
    return;
  }
  const auto context = current();
  const bool keepScale = context && baseline && scaleOrigin && same_context(*context,*baseline) &&
    same_context(*context,*scaleOrigin) && Postprocessor::same_deformation(context->settings,baseline->settings);
  if (!keepScale)
  {
    scaleTimer->stop();
    scaleOrigin.reset();
  }
  const bool changedOwner = !context || !baseline || !same_context(*context,*baseline) ||
    !Postprocessor::same_deformation(context->settings,baseline->settings);
  loading = true;
  setEnabled(context.has_value());
  baseline = context;
  if (context)
  {
    populate_fields(*context,context->settings);
    enabled->setChecked(context->settings.enabled);
    overlay->setChecked(context->settings.overlay);
    if (!keepScale)
    {
      scale->setText(QString::fromStdString(number(context->settings.scale)));
    }
  }
  else
  {
    fields->clear();
    catalogue.clear();
    enabled->setChecked(false);
    overlay->setChecked(false);
    scale->setText(QString::fromStdString(number(Postprocessor::DeformationDefaults::scale)));
  }
  if (changedOwner)
  {
    show_error({});
  }
  loading = false;
}

bool PostprocessorDeformationControls::owned() const
{
  const auto context = current();
  return baseline && context && same_context(*baseline,*context) &&
    Postprocessor::same_deformation(baseline->settings,context->settings);
}

bool PostprocessorDeformationControls::dispatch(const std::string& operation,
  const Postprocessor::ResultDeformation& expected, bool preview, std::string& error)
{
  error.clear();
  if (!owned())
  {
    error = "The deformation target changed; the pending edit was discarded.";
    return false;
  }
  const auto owner = *baseline;
  const auto source = quote(owner.name);
  const auto& callback = preview ? silent : submit;
  if (source.empty() || !callback)
  {
    error = "The deformation command callback or quoted source name is unavailable.";
    return false;
  }
  dispatching = true;
  bool accepted = false;
  try
  {
    accepted = callback("post deform "+operation+" source "+source,error);
    const auto after = current();
    if (accepted && (!after || !same_context(owner,*after) ||
        !Postprocessor::same_deformation(after->settings,expected)))
    {
      accepted = false;
      error = "The deformation command did not publish the expected settings for this target.";
    }
    if (accepted)
    {
      baseline = after;
    }
  }
  catch (const std::exception& exception)
  {
    accepted = false;
    error = exception.what();
  }
  catch (...)
  {
    accepted = false;
    error = "Cannot change deformation: unknown failure.";
  }
  dispatching = false;
  if (!accepted && error.empty())
  {
    error = "The deformation command was rejected; current settings were reloaded.";
  }
  return accepted;
}

void PostprocessorDeformationControls::change_flag(bool deformation, bool value)
{
  if (loading || dispatching)
  {
    return;
  }
  const auto owner = baseline;
  finish_scale();
  std::string error;
  if (!owner || !baseline || !same_context(*owner,*baseline) || !owned())
  {
    refresh();
    return;
  }
  auto target = baseline->settings;
  auto& flag = deformation ? target.enabled : target.overlay;
  if (flag != value)
  {
    flag = value;
    const auto operation = std::string(deformation ? "" : "overlay ")+(value ? "on" : "off");
    dispatch(operation,target,false,error);
  }
  scaleOrigin.reset();
  baseline.reset();
  refresh();
  show_error(error);
}

void PostprocessorDeformationControls::select_field(int index)
{
  if (loading || dispatching || !baseline)
  {
    return;
  }
  if (index < 0 || index >= fields->count())
  {
    refresh();
    return;
  }
  const auto owner = *baseline;
  auto selection = owner.settings;
  const int item = fields->itemData(index).toInt();
  std::string operation;
  if (item == -1)
  {
    selection.field_mode = Postprocessor::DeformationFieldMode::Automatic;
    operation = "field auto";
  }
  else if (item >= 0 && static_cast<std::size_t>(item) < catalogue.size())
  {
    const auto& group = catalogue[static_cast<std::size_t>(item)];
    auto dataset = selection.field && selection.field->group_key == group.key ?
      selection.field->dataset_id : std::optional<std::size_t>{};
    if (!dataset && std::count_if(catalogue.begin(),catalogue.end(),[&](const Postprocessor::ResultFieldGroup& other)
        {
          return other.field.array_name == group.field.array_name && Postprocessor::deformation_vector(other.field);
        }) > 1)
    {
      dataset = group.field.dataset_id;
    }
    const auto argument = quote(group.field.array_name);
    if (!argument.empty())
    {
      selection.field_mode = Postprocessor::DeformationFieldMode::Explicit;
      selection.field = Postprocessor::FieldSelection{dataset,group.field.array_name,group.field.association,{},true,group.key};
      operation = "field "+argument+(dataset ? " dataset "+std::to_string(*dataset) : "");
    }
  }
  finish_scale();
  std::string error;
  if (operation.empty())
  {
    error = "Choose an available vector with a representable quoted name.";
  }
  else if (baseline && same_context(owner,*baseline) && owned())
  {
    auto target = baseline->settings;
    target.field_mode = selection.field_mode;
    target.field = selection.field;
    if (!Postprocessor::same_deformation(target,baseline->settings))
    {
      dispatch(operation,target,false,error);
    }
  }
  baseline.reset();
  refresh();
  show_error(error);
}

bool PostprocessorDeformationControls::read_scale(double& value) const
{
  bool ok = false;
  auto locale = QLocale::c();
  locale.setNumberOptions(QLocale::RejectGroupSeparator);
  value = locale.toDouble(scale->text(),&ok);
  return ok && scale->hasAcceptableInput() && std::isfinite(value) && value >= 0;
}

void PostprocessorDeformationControls::edit_scale()
{
  if (loading || dispatching)
  {
    return;
  }
  if (!owned())
  {
    cancel_scale();
    refresh();
    return;
  }
  if (!scaleOrigin)
  {
    scaleOrigin = baseline;
  }
  scaleTimer->stop();
  double value;
  if (read_scale(value))
  {
    scaleTimer->start();
  }
  show_error({});
}

void PostprocessorDeformationControls::preview_scale()
{
  scaleTimer->stop();
  if (!scaleOrigin)
  {
    return;
  }
  if (!owned() || !same_context(*scaleOrigin,*baseline))
  {
    cancel_scale();
    refresh();
    return;
  }
  double value;
  if (!read_scale(value) || value == baseline->settings.scale)
  {
    return;
  }
  auto target = baseline->settings;
  target.scale = value;
  std::string error;
  if (!dispatch("scale "+number(value),target,true,error))
  {
    scaleOrigin.reset();
    baseline.reset();
    refresh();
    show_error(error);
  }
}

void PostprocessorDeformationControls::finish_scale()
{
  if (loading || dispatching || !scaleOrigin)
  {
    return;
  }
  scaleTimer->stop();
  if (!owned() || !same_context(*scaleOrigin,*baseline))
  {
    cancel_scale();
    refresh();
    return;
  }
  double value;
  if (!read_scale(value))
  {
    // An incomplete exponent does not replace the last successfully displayed value.
    value = baseline->settings.scale;
  }
  const bool changed = value != scaleOrigin->settings.scale;
  const auto origin = *scaleOrigin;
  scaleOrigin.reset();
  auto target = baseline->settings;
  target.scale = value;
  std::string error;
  if (changed || value != baseline->settings.scale)
  {
    // A preview may already have published this value. Still submit the final
    // normal command once for replay. A net-zero edit only restores silently.
    if (!dispatch("scale "+number(value),target,!changed,error) && owned() && same_context(origin,*baseline))
    {
      auto restore = baseline->settings;
      restore.scale = origin.settings.scale;
      std::string restoreError;
      if (!dispatch("scale "+number(restore.scale),restore,true,restoreError))
      {
        error += " Restore failed: "+restoreError;
      }
    }
  }
  baseline.reset();
  refresh();
  show_error(error);
}

void PostprocessorDeformationControls::cancel_scale()
{
  scaleTimer->stop();
  const auto origin = scaleOrigin;
  scaleOrigin.reset();
  if (origin && owned() && same_context(*origin,*baseline) && baseline->settings.scale != origin->settings.scale)
  {
    auto target = baseline->settings;
    target.scale = origin->settings.scale;
    std::string error;
    dispatch("scale "+number(target.scale),target,true,error);
    show_error(error);
  }
}

bool PostprocessorDeformationControls::eventFilter(QObject* watched, QEvent* event)
{
  if (watched == scale && event->type() == QEvent::KeyPress)
  {
    const auto* key = static_cast<QKeyEvent*>(event);
    if (key->key() == Qt::Key_Return || key->key() == Qt::Key_Enter)
    {
      finish_scale();
      return true;
    }
  }
  if (watched == scale && event->type() == QEvent::FocusOut)
  {
    // QLineEdit does not emit editingFinished for incomplete numeric input.
    finish_scale();
  }
  return QWidget::eventFilter(watched,event);
}

void PostprocessorDeformationControls::hideEvent(QHideEvent* event)
{
  cancel_scale();
  refresh();
  QWidget::hideEvent(event);
}

void PostprocessorDeformationControls::show_error(const std::string& error)
{
  errorLabel->setText(error.empty() ? QString() : tr("Error"));
  errorLabel->setToolTip(QString::fromUtf8(error.c_str()));
  errorLabel->setAccessibleName(QString::fromUtf8(error.c_str()));
}
