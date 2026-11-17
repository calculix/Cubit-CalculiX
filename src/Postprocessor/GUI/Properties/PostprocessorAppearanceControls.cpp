#include "PostprocessorAppearanceControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/display/AppearanceCommandText.hpp"
#include "Postprocessor/GUI/ResultsTree/PostprocessorStyleDelegate.hpp"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QColorDialog>
#include <QPixmap>
#include <QStandardItemModel>
#include <algorithm>
#include <utility>
PostprocessorAppearanceControls::PostprocessorAppearanceControls(std::weak_ptr<PostprocessorApplicationInterface> app,
  Submit dispatcher,QWidget* parent) : QWidget(parent),application(std::move(app)),submit(std::move(dispatcher))
{
  setObjectName("Postprocessor.Appearance");
  auto* form = new QFormLayout(this);
  selected = new QLabel(this);
  selected->setWordWrap(true);
  selected->setTextFormat(Qt::PlainText);
  form->addRow(tr("Selected"),selected);
  style = new QComboBox(this);
  style->setObjectName("Postprocessor.Appearance.Style");
  style->addItem(tr("Mixed / unchanged"),-1);
  for (auto option : {Postprocessor::DisplayStyle::Wireframe,Postprocessor::DisplayStyle::HiddenLine,
      Postprocessor::DisplayStyle::Shaded,Postprocessor::DisplayStyle::Transparent,
      Postprocessor::DisplayStyle::Points,Postprocessor::DisplayStyle::RoundedPoints})
  {
    style->addItem(appearance_style_icon(option),appearance_style_label(option),static_cast<int>(option));
  }
  form->addRow(tr("Style"),style);
  color = new QPushButton(this);
  color->setObjectName("Postprocessor.Appearance.Color");
  color->setToolTip(tr("Solid colors apply when no result scalar colors are displayed."));
  native = new QPushButton(tr("Native"),this);
  auto* colors = new QHBoxLayout;
  colors->addWidget(color);
  colors->addWidget(native);
  form->addRow(tr("Solid color"),colors);
  edges = new QCheckBox(tr("Element edges"),this);
  edges->setObjectName("Postprocessor.Appearance.Edges");
  edges->setTristate(true);
  form->addRow(edges);
  opacity = new QDoubleSpinBox(this);
  opacity->setObjectName("Postprocessor.Appearance.Opacity");
  opacity->setRange(1,100);
  opacity->setSuffix(" %");
  form->addRow(tr("Transparent opacity"),opacity);
  pointSize = new QDoubleSpinBox(this);
  pointSize->setObjectName("Postprocessor.Appearance.PointSize");
  pointSize->setRange(1,100);
  pointSize->setSuffix(" px");
  form->addRow(tr("Point size"),pointSize);
  auto* buttons = new QHBoxLayout;
  resetButton = new QPushButton(tr("Reset"),this);
  applyButton = new QPushButton(tr("Apply"),this);
  applyButton->setObjectName("Postprocessor.Appearance.Apply");
  buttons->addWidget(resetButton);
  buttons->addWidget(applyButton);
  form->addRow(buttons);
  errorLabel = new QLabel(this);
  errorLabel->setWordWrap(true);
  errorLabel->setTextFormat(Qt::PlainText);
  form->addRow(errorLabel);
  connect(style,&QComboBox::activated,this,[this](int)
  {
    if (!loading)
    {
      edited.insert("style");
      update_enabled();
    }
  });
  connect(edges,&QCheckBox::checkStateChanged,this,[this](Qt::CheckState)
  {
    if (!loading)
    {
      edited.insert("edges");
    }
  });
  connect(opacity,&QDoubleSpinBox::valueChanged,this,[this](double)
  {
    if (!loading)
    {
      edited.insert("opacity");
    }
  });
  connect(pointSize,&QDoubleSpinBox::valueChanged,this,[this](double)
  {
    if (!loading)
    {
      edited.insert("pointsize");
    }
  });
  connect(color,&QPushButton::clicked,this,[this]()
  {
    const auto chosen = QColorDialog::getColor(QColor::fromRgbF(chosenColor[0],chosenColor[1],chosenColor[2]),this,tr("Solid color"));
    if (chosen.isValid())
    {
      chosenColor = {chosen.redF(),chosen.greenF(),chosen.blueF()};
      colorChange = Postprocessor::ColorChange::Override;
      edited.insert("color");
      color->setText(chosen.name());
      QPixmap swatch(16,16);
      swatch.fill(chosen);
      color->setIcon(QIcon(swatch));
    }
  });
  connect(native,&QPushButton::clicked,this,[this]()
  {
    colorChange = Postprocessor::ColorChange::Native;
    edited.insert("color");
    color->setText(tr("Native"));
    color->setIcon(QIcon());
  });
  connect(applyButton,&QPushButton::clicked,this,[this]()
  {
    apply(false);
  });
  connect(resetButton,&QPushButton::clicked,this,[this]()
  {
    apply(true);
  });
  refresh();
}
void PostprocessorAppearanceControls::set_targets(const std::vector<Postprocessor::AppearanceTarget>& value)
{
  const bool same = targets.size() == value.size() && std::equal(targets.begin(),targets.end(),value.begin(),[](const auto& a,const auto& b)
  {
    return a.source_id == b.source_id && a.view_id == b.view_id && a.dataset_id == b.dataset_id;
  });
  if (!same)
  {
    targets = value;
    edited.clear();
    baseline.clear();
  }
  refresh();
}
void PostprocessorAppearanceControls::refresh()
{
  const auto app = application.lock();
  setEnabled(app && !targets.empty());
  if (!app || targets.empty())
  {
    selected->setText(tr("Select blocks, node sets or side sets in the source tree."));
    return;
  }
  std::vector<Postprocessor::DatasetAppearance> values;
  QStringList names;
  bool valid = true;
  for (const auto& target : targets)
  {
    bool found = false;
    for (const auto& dataset : app->mesh_datasets(target.source_id))
    {
      if (dataset.id == target.dataset_id)
      {
        QString label = QString::fromStdString(dataset.name);
        const auto source = app->session().sources.find(target.source_id);
        if (source != app->session().sources.end())
        {
          label = QString::fromStdString(source->second.name)+" / "+label;
        }
        if (dataset.model_id >= 0)
        {
          label += tr(" [ID %1]").arg(dataset.model_id);
        }
        names.append(label);
        found = true;
      }
    }
    valid = valid && found && target.view_id == app->session().active_view_id;
    values.push_back(app->dataset_appearance(target.source_id,target.view_id,target.dataset_id));
  }
  setEnabled(valid);
  selected->setText(tr("%1 items: %2").arg(targets.size()).arg(names.join(", ")));
  if (!valid)
  {
    return;
  }
  const bool unchanged = baseline.size() == values.size() && std::equal(baseline.begin(),baseline.end(),values.begin(),Postprocessor::same_appearance);
  if (unchanged && !edited.empty())
  {
    return;
  }
  loading = true;
  baseline = values;
  edited.clear();
  colorChange = Postprocessor::ColorChange::Unchanged;
  const auto& first = values.front();
  bool sameStyle = true,sameEdges = true,sameColor = true,sameOpacity = true,sameSize = true;
  for (const auto& value : values)
  {
    sameStyle = sameStyle && value.style == first.style;
    sameEdges = sameEdges && value.edges == first.edges;
    sameColor = sameColor && value.solid_color == first.solid_color;
    sameOpacity = sameOpacity && value.opacity == first.opacity;
    sameSize = sameSize && value.point_size == first.point_size;
  }
  style->setCurrentIndex(0);
  if (sameStyle)
  {
    style->setCurrentIndex(style->findData(static_cast<int>(first.style)));
  }
  auto* model = qobject_cast<QStandardItemModel*>(style->model());
  for (int index = 1; index < style->count(); ++index)
  {
    bool supported = true;
    for (const auto& target : targets)
    {
      for (const auto& dataset : app->mesh_datasets(target.source_id))
      {
        if (dataset.id == target.dataset_id)
        {
          supported = supported && Postprocessor::supports_style(dataset,static_cast<Postprocessor::DisplayStyle>(style->itemData(index).toInt()));
        }
      }
    }
    if (model)
    {
      model->item(index)->setEnabled(supported);
    }
  }
  edges->setCheckState(Qt::PartiallyChecked);
  if (sameEdges)
  {
    edges->setCheckState(Qt::Unchecked);
    if (first.edges)
    {
      edges->setCheckState(Qt::Checked);
    }
  }
  opacity->setValue(first.opacity*100);
  opacity->setToolTip(tr("Mixed values remain unchanged unless edited."));
  pointSize->setValue(first.point_size);
  opacity->setSpecialValueText(QString());
  pointSize->setSpecialValueText(QString());
  if (!sameOpacity)
  {
    opacity->findChild<QLineEdit*>()->clear();
  }
  if (!sameSize)
  {
    pointSize->findChild<QLineEdit*>()->clear();
  }
  color->setText(tr("Mixed"));
  if (sameColor)
  {
    color->setText(tr("Native"));
    chosenColor = Postprocessor::DisplayDefaults::surface_color;
    for (const auto& dataset : app->mesh_datasets(targets.front().source_id))
    {
      if (dataset.id == targets.front().dataset_id && dataset.solid_color)
      {
        chosenColor = *dataset.solid_color;
      }
    }
    if (first.solid_color)
    {
      chosenColor = *first.solid_color;
      color->setText(QColor::fromRgbF(chosenColor[0],chosenColor[1],chosenColor[2]).name());
    }
  }
  color->setIcon(QIcon());
  if (sameColor && (first.solid_color || targets.size() == 1))
  {
    QPixmap swatch(16,16);
    swatch.fill(QColor::fromRgbF(chosenColor[0],chosenColor[1],chosenColor[2]));
    color->setIcon(QIcon(swatch));
  }
  loading = false;
  update_enabled();
}
void PostprocessorAppearanceControls::update_enabled()
{
  const int selectedStyle = style->currentData().toInt();
  const bool mixed = selectedStyle < 0;
  const auto selected = static_cast<Postprocessor::DisplayStyle>(selectedStyle);
  opacity->setEnabled(mixed || selected == Postprocessor::DisplayStyle::Transparent);
  pointSize->setEnabled(mixed || selected == Postprocessor::DisplayStyle::Points || selected == Postprocessor::DisplayStyle::RoundedPoints);
  edges->setEnabled(mixed || selected == Postprocessor::DisplayStyle::Shaded || selected == Postprocessor::DisplayStyle::Transparent);
}
void PostprocessorAppearanceControls::apply(bool reset)
{
  const auto app = application.lock();
  if (!app || !submit)
  {
    return;
  }
  Postprocessor::AppearancePatch patch;
  patch.reset = reset;
  if (!reset)
  {
    if (edited.count("style") && style->currentData().toInt() >= 0)
    {
      patch.style = static_cast<Postprocessor::DisplayStyle>(style->currentData().toInt());
    }
    if (edited.count("edges") && edges->checkState() != Qt::PartiallyChecked)
    {
      patch.edges = edges->isChecked();
    }
    if (edited.count("opacity"))
    {
      patch.opacity = opacity->value()/100.0;
    }
    if (edited.count("pointsize"))
    {
      patch.point_size = pointSize->value();
    }
    if (edited.count("color"))
    {
      patch.color_change = colorChange; patch.color = chosenColor;
    }
  }
  std::string error;
  if (!app->change_dataset_appearance(targets,patch,error,true))
  {
    errorLabel->setText(QString::fromStdString(error));
    return;
  }
  const auto commands = Postprocessor::appearance_commands(*app,targets,patch,error);
  for (const auto& command : commands)
  {
    if (!submit(command.toStdString(),error))
    {
      break;
    }
  }
  errorLabel->setText(QString::fromStdString(error));
  edited.clear();
  baseline.clear();
  refresh();
}
