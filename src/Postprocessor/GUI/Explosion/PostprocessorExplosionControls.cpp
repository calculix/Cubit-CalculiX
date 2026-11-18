#include "PostprocessorExplosionControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include "CubitMessage.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QScopedValueRollback>
#include <QSlider>
#include <QStringList>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <utility>
namespace
{
  QString quote(const QString& value)
  {
    QChar delimiter('"');
    if (value.contains(delimiter))
    {
      delimiter = QChar(0x27);
    }
    if (value.contains(delimiter) || value.contains('\n') || value.contains('\r'))
    {
      return {};
    }
    return delimiter+value+delimiter;
  }
}
PostprocessorExplosionControls::PostprocessorExplosionControls(std::weak_ptr<PostprocessorApplicationInterface> app,
  Dispatch dispatch,Dispatch silent,QWidget* parent)
  : QWidget(parent),application(std::move(app)),submit(std::move(dispatch)),preview(std::move(silent))
{
  auto* layout = new QFormLayout(this);
  sources = new QComboBox(this);
  blocks = new QComboBox(this);
  grouping = new QComboBox(this);
  direction = new QComboBox(this);
  grouping->addItems({tr("Automatic"),tr("Sources"),tr("Blocks")});
  direction->addItems({tr("Automatic"),tr("X"),tr("Y"),tr("Z"),tr("Radial")});
  layout->addRow(tr("Source"),sources);
  layout->addRow(tr("Mesh block / offset"),blocks);
  layout->addRow(tr("Grouping"),grouping);
  layout->addRow(tr("Direction"),direction);
  components = new QCheckBox(tr("Explode components"),this);
  mesh = new QCheckBox(tr("Explode mesh"),this);
  amount = new QDoubleSpinBox(this);
  meshAmount = new QDoubleSpinBox(this);
  for (auto* spin : {amount,meshAmount})
  {
    spin->setRange(0,100);
    spin->setDecimals(3);
    spin->setSingleStep(.05);
    spin->setKeyboardTracking(false);
  }
  componentSlider = new QSlider(Qt::Horizontal,this);
  meshSlider = new QSlider(Qt::Horizontal,this);
  componentSlider->setRange(0,300);
  meshSlider->setRange(0,100);
  componentSlider->setToolTip(tr("Component separation; numeric input supports values up to 100"));
  meshSlider->setToolTip(tr("Element separation; numeric input supports values up to 100"));
  layout->addRow(components);
  layout->addRow(tr("Amount"),amount);
  layout->addRow(componentSlider);
  layout->addRow(mesh);
  layout->addRow(tr("Mesh amount"),meshAmount);
  layout->addRow(meshSlider);
  pulse = new QCheckBox(tr("Pulse explosion"),this);
  pulse->setObjectName("Postprocessor.Explosion.Pulse");
  pulse->setToolTip(tr("Animate the enabled explosion modes without changing their configured amounts"));
  pulseTarget = new QComboBox(this);
  pulseTarget->addItems({tr("Components"),tr("Mesh"),tr("Both")});
  pulsePeriod = new QDoubleSpinBox(this);
  pulsePeriod->setRange(0.1,3600);
  pulsePeriod->setSuffix(tr(" s"));
  pulsePeriod->setDecimals(2);
  pulsePeriod->setKeyboardTracking(false);
  pulseMinimum = new QDoubleSpinBox(this);
  pulseMaximum = new QDoubleSpinBox(this);
  for (auto* spin : {pulseMinimum,pulseMaximum})
  {
    spin->setRange(0,1);
    spin->setDecimals(2);
    spin->setSingleStep(.05);
    spin->setKeyboardTracking(false);
  }
  layout->addRow(pulse);
  layout->addRow(tr("Pulse target"),pulseTarget);
  layout->addRow(tr("Pulse period"),pulsePeriod);
  layout->addRow(tr("Minimum multiplier"),pulseMinimum);
  layout->addRow(tr("Maximum multiplier"),pulseMaximum);
  connect(pulse,&QCheckBox::clicked,this,[this](bool enabled)
  {
    QString operation = "off";
    if (enabled)
    {
      operation = "on";
    }
    pulse_command(operation);
  });
  connect(pulseTarget,&QComboBox::activated,this,[this](int index)
  {
    const QStringList targets = {"components","mesh","both"};
    if (index >= 0 && index < targets.size())
    {
      pulse_command("target "+targets[index]);
    }
  });
  connect(pulsePeriod,&QDoubleSpinBox::valueChanged,this,[this](double)
  {
    pulse_command("period "+QString::number(pulsePeriod->value(),'g',17));
  });
  for (auto* spin : {pulseMinimum,pulseMaximum})
  {
    connect(spin,&QDoubleSpinBox::valueChanged,this,[this](double)
    {
      pulse_command("range "+QString::number(pulseMinimum->value(),'g',17)+" "+QString::number(pulseMaximum->value(),'g',17));
    });
  }
  gap = new QDoubleSpinBox(this);
  gap->setRange(0,1);
  gap->setDecimals(3);
  gap->setSingleStep(.01);
  gap->setKeyboardTracking(false);
  gap->setToolTip(tr("Gap as a fraction of the undeformed assembly diagonal"));
  layout->addRow(tr("Layout gap"),gap);
  offsetUnit = new QLabel(this);
  layout->addRow(offsetUnit);
  const char* axes[] = {"X","Y","Z"};
  for (std::size_t i = 0; i < offsets.size(); ++i)
  {
    offsets[i] = new QDoubleSpinBox(this);
    offsets[i]->setRange(-1e15,1e15);
    offsets[i]->setDecimals(6);
    layout->addRow(tr("Manual offset %1").arg(axes[i]),offsets[i]);
  }
  auto* offset = new QPushButton(tr("Apply block offset"),this);
  layout->addRow(offset);
  auto* automatic = new QPushButton(tr("Explode automatically"),this);
  auto* recalculate = new QPushButton(tr("Recalculate layout"),this);
  auto* reset = new QPushButton(tr("Reset explosion"),this);
  layout->addRow(automatic);
  layout->addRow(recalculate);
  layout->addRow(reset);
  auto* hint = new QLabel(tr("Cuts are applied before explosion. Hide explosion to edit the cutting-plane outline."),this);
  hint->setWordWrap(true);
  layout->addRow(hint);
  previewTimer = new QTimer(this);
  previewTimer->setSingleShot(true);
  previewTimer->setInterval(Postprocessor::ExplosionDefaults::preview_delay_ms);
  connect(previewTimer,&QTimer::timeout,this,[this]()
  {
    if (!pending.isEmpty() && preview)
    {
      preview(pending);
    }
  });
  connect(sources,&QComboBox::activated,this,[this](int)
  {
    discard_pending();
    refresh();
  });
  connect(blocks,&QComboBox::activated,this,[this](int)
  {
    discard_pending();
    refresh();
  });
  connect(components,&QCheckBox::clicked,this,[this](bool on)
  {
    QString operation = "off";
    if (on)
    {
      operation = "on";
    }
    submit("post explode components "+operation+scope());
  });
  connect(mesh,&QCheckBox::clicked,this,[this](bool on)
  {
    QString operation = "off";
    if (on)
    {
      operation = "on";
    }
    QString block;
    if (!blocks->currentData().toString().isEmpty())
    {
      block = " block "+quote(blocks->currentData().toString());
    }
    submit("post explode mesh "+operation+block+scope());
  });
  connect(grouping,&QComboBox::activated,this,[this](int index)
  {
    const char* values[] = {"auto","sources","blocks"};
    submit("post explode grouping "+QString::fromLatin1(values[index])+scope());
  });
  connect(direction,&QComboBox::activated,this,[this](int index)
  {
    const char* values[] = {"auto","x","y","z","radial"};
    submit("post explode direction "+QString::fromLatin1(values[index])+scope());
  });
  connect(gap,&QDoubleSpinBox::editingFinished,this,[this]()
  {
    if (!refreshing)
    {
      submit("post explode gap "+QString::number(gap->value(),'g',17)+scope());
    }
  });
  const auto connectAmount = [this](QSlider* slider,QDoubleSpinBox* spin,bool isMesh)
  {
    connect(slider,&QSlider::sliderPressed,this,[this]()
    {
      motionScope = scope();
    });
    connect(slider,&QSlider::valueChanged,this,[this,spin,isMesh](int value)
    {
      if (!refreshing)
      {
        spin->setValue(value/100.0);
        amount_changed(isMesh,false);
      }
    });
    connect(slider,&QSlider::sliderReleased,this,[this,isMesh]()
    {
      amount_changed(isMesh,true);
    });
    connect(slider,&QSlider::actionTriggered,this,[this,slider,isMesh](int)
    {
      if (!refreshing && !slider->isSliderDown())
      {
        QTimer::singleShot(0,this,[this,isMesh]()
        {
          amount_changed(isMesh,true);
        });
      }
    });
    connect(spin,&QDoubleSpinBox::editingFinished,this,[this,isMesh]()
    {
      if (!refreshing)
      {
        amount_changed(isMesh,true);
      }
    });
  };
  connectAmount(componentSlider,amount,false);
  connectAmount(meshSlider,meshAmount,true);
  connect(automatic,&QPushButton::clicked,this,[this]()
  {
    submit("post explode"+scope());
  });
  connect(recalculate,&QPushButton::clicked,this,[this]()
  {
    submit("post explode recalculate"+scope());
  });
  connect(reset,&QPushButton::clicked,this,[this]()
  {
    discard_pending();
    submit("post explode reset"+scope());
  });
  connect(offset,&QPushButton::clicked,this,[this]()
  {
    if (sources->currentData().toString().isEmpty() || blocks->currentData().toString().isEmpty())
    {
      return;
    }
    QString command = "post explode offset block "+quote(blocks->currentData().toString());
    for (const auto* spin : offsets)
    {
      command += " "+QString::number(spin->value(),'g',17);
    }
    submit(command+scope());
  });
  refresh();
}
QString PostprocessorExplosionControls::scope() const
{
  const auto app = application.lock();
  if (!app)
  {
    return {};
  }
  QString result = " view "+quote(QString::fromStdString(app->session().active_view_id));
  if (!sources->currentData().toString().isEmpty())
  {
    result = " source "+quote(sources->currentData().toString())+result;
  }
  return result;
}
void PostprocessorExplosionControls::pulse_command(const QString& operation)
{
  const auto app = application.lock();
  if (refreshing || !app || !submit)
  {
    return;
  }
  if (operation.startsWith("range ") && pulseMinimum->value() > pulseMaximum->value())
  {
    PRINT_ERROR("Pulse minimum must not exceed maximum.\n");
    return;
  }
  discard_pending();
  submit("post explode pulse "+operation+" view "+quote(QString::fromStdString(app->session().active_view_id)));
}
void PostprocessorExplosionControls::amount_changed(bool isMesh,bool final)
{
  QString command = "post explode amount ";
  double value = amount->value();
  if (isMesh)
  {
    command = "post explode mesh amount ";
    value = meshAmount->value();
  }
  command += QString::number(value,'g',17);
  if (isMesh && !blocks->currentData().toString().isEmpty())
  {
    command += " block "+quote(blocks->currentData().toString());
  }
  if (motionScope.isEmpty())
  {
    command += scope();
  }
  else
  {
    command += motionScope;
  }
  if (final)
  {
    discard_pending();
    submit(command);
  }
  else
  {
    pending = command;
    previewTimer->start();
  }
}
void PostprocessorExplosionControls::discard_pending()
{
  previewTimer->stop();
  pending.clear();
  motionScope.clear();
}
void PostprocessorExplosionControls::refresh()
{
  if (componentSlider->isSliderDown() || meshSlider->isSliderDown() || previewTimer->isActive())
  {
    return;
  }
  const QScopedValueRollback<bool> guard(refreshing,true);
  const auto app = application.lock();
  setEnabled(static_cast<bool>(app));
  if (!app)
  {
    return;
  }
  const auto& state = app->session();
  const auto view = state.views.find(state.active_view_id);
  if (view == state.views.end())
  {
    setEnabled(false);
    return;
  }
  const auto previous = sources->currentData();
  sources->clear();
  sources->addItem(tr("All visible sources"),QString());
  for (const auto& entry : state.sources)
  {
    const auto name = QString::fromUtf8(entry.second.name.c_str());
    if (!quote(name).isEmpty())
    {
      sources->addItem(name,name);
    }
  }
  sources->setCurrentIndex(std::max(0,sources->findData(previous)));
  const auto previousBlock = blocks->currentData();
  blocks->clear();
  blocks->addItem(tr("All blocks"),QString());
  std::string selectedSource;
  for (const auto& entry : state.sources)
  {
    if (QString::fromUtf8(entry.second.name.c_str()) == sources->currentData().toString())
    {
      selectedSource = entry.first;
      for (const auto& dataset : app->mesh_datasets(entry.first))
      {
        if (Postprocessor::dataset_in_category(dataset,Postprocessor::DatasetCategory::Blocks))
        {
          const auto name = QString::fromUtf8(dataset.name.c_str());
          if (!quote(name).isEmpty())
          {
            blocks->addItem(name,name);
          }
        }
      }
    }
  }
  blocks->setCurrentIndex(std::max(0,blocks->findData(previousBlock)));
  const auto& settings = view->second.explosion;
  pulse->setChecked(settings.pulse.enabled);
  pulsePeriod->setValue(settings.pulse.period_seconds);
  pulseMinimum->setValue(settings.pulse.minimum);
  pulseMaximum->setValue(settings.pulse.maximum);
  pulseTarget->setCurrentIndex(static_cast<int>(settings.pulse.target));
  components->setChecked(settings.enabled);
  amount->setValue(settings.amount);
  componentSlider->setValue(static_cast<int>(std::round(settings.amount*100)));
  gap->setValue(settings.gap);
  grouping->setCurrentIndex(static_cast<int>(settings.grouping));
  direction->setCurrentIndex(static_cast<int>(settings.direction));
  Postprocessor::MeshExplosion meshSettings;
  if (!selectedSource.empty())
  {
    const auto found = settings.meshes.find(selectedSource);
    if (found != settings.meshes.end())
    {
      meshSettings = found->second;
    }
  }
  else
  {
    for (const auto& item : settings.meshes)
    {
      if (item.second.enabled)
      {
        meshSettings = item.second;
        break;
      }
    }
  }
  mesh->setChecked(meshSettings.enabled);
  meshAmount->setValue(meshSettings.amount);
  meshSlider->setValue(static_cast<int>(std::round(meshSettings.amount*100)));
  QString unit = tr("native / uncalibrated length");
  if (!view->second.scene_unit.empty())
  {
    unit = QString::fromStdString(view->second.scene_unit);
  }
  offsetUnit->setText(tr("Offsets [%1]").arg(unit));
  for (auto* spin : offsets)
  {
    spin->setValue(0);
  }
  for (const auto& piece : settings.placements)
  {
    if (piece.source_id == selectedSource && QString::fromUtf8(piece.block.name.c_str()) == blocks->currentData().toString())
    {
      for (std::size_t a = 0; a < offsets.size(); ++a)
      {
        offsets[a]->setValue(piece.manual[a]);
      }
    }
  }
}
