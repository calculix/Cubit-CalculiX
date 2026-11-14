#include <utility>
#include "PostprocessorSectionControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/filters/FilterCommandText.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QDoubleValidator>
#include <QLocale>
#include <QTimer>
#include <QMessageBox>
#include <cmath>
PostprocessorSectionControls::PostprocessorSectionControls(std::weak_ptr<PostprocessorApplicationInterface> value,std::function<void(const QString&)> dispatch,QWidget* parent)
  : QWidget(parent),application(std::move(value)),submit(std::move(dispatch))
{
  auto* form = new QFormLayout(this);
  filters = new QComboBox(this);
  form->addRow(tr("Section"),filters);
  name = new QLineEdit(this);
  form->addRow(tr("Name"),name);
  const QString labels[] = {tr("Origin X"),tr("Origin Y"),tr("Origin Z"),tr("Normal X"),tr("Normal Y"),tr("Normal Z")};
  for (int i = 0; i < 6; ++i)
  {
    coordinates[i] = new QLineEdit(this);
    auto* validator = new QDoubleValidator(coordinates[i]);
    validator->setLocale(QLocale::c());
    coordinates[i]->setValidator(validator);
    coordinates[i]->setText(QString::number(i < 3 ? Postprocessor::FilterDefaults::origin[i] : Postprocessor::FilterDefaults::normal[i-3],'g',17));
    form->addRow(labels[i],coordinates[i]);
    connect(coordinates[i],&QLineEdit::editingFinished,this,[this]()
    {
      if (!loading && !filters->currentData().toString().isEmpty())
      {
        submit_plane(false);
      }
    });
  }
  side = new QComboBox(this);
  side->addItems({tr("Positive"),tr("Negative")});
  side->hide();

  visible = new QCheckBox(tr("Show in active view"),this);
  form->addRow(visible);
  auto* create = new QPushButton(tr("Create section"),this);
  form->addRow(create);
  auto* remove = new QPushButton(tr("Remove selected filter"),this);
  form->addRow(remove);
  connect(create,&QPushButton::clicked,this,[this]()
  {
    submit_plane(true);
  });
  connect(remove,&QPushButton::clicked,this,[this]()
  {
    if (!filters->currentData().toString().isEmpty())
    {
      send("post filter remove "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString()));
    }
  });
  connect(name,&QLineEdit::editingFinished,this,[this]()
  {
    if (!loading && !filters->currentData().toString().isEmpty() && name->text() != filters->currentText())
    {
      send("post filter rename "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString())+
        " "+Postprocessor::plot_quote(name->text().toUtf8().toStdString()));
    }
  });
  connect(filters,&QComboBox::activated,this,[this](int)
  {
    if (!filters->currentData().toString().isEmpty())
    {
      send("post filter show "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString()));
    }
    else
    {
      name->clear();
    }
    refresh();
  });
  connect(visible,&QCheckBox::toggled,this,[this](bool checked)
  {
    if (!loading && !filters->currentData().toString().isEmpty())
    {
      send(QString(checked ? "post filter show " : "post filter hide ")+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString()));
    }
  });
  connect(side,&QComboBox::activated,this,[this](int index)
  {
    if (!loading && !filters->currentData().toString().isEmpty())
    {
      send("post clip keep "+Postprocessor::plot_quote(filters->currentText().toUtf8().toStdString())+(index == 0 ? " positive" : " negative"));
    }
  });
  refresh();
}
void PostprocessorSectionControls::send(const QString& command)
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
void PostprocessorSectionControls::submit_plane(bool create)
{
  const auto app = application.lock();
  if (!app)
  {
    return;
  }
  Postprocessor::PlaneDefinition plane;
  for (int i = 0; i < 6; ++i)
  {
    bool valid = false;
    const auto value = QLocale::c().toDouble(coordinates[i]->text(),&valid);
    if (!valid || !std::isfinite(value))
    {
      QMessageBox::warning(this,tr("Plane"),tr("Enter finite origin and normal coordinates."));
      return;
    }
    (i < 3 ? plane.origin[i] : plane.normal[i-3]) = value;
  }
  const auto chosenName = create ? (name->text().isEmpty() ? app->suggested_filter_name(Postprocessor::FilterKind::Section) : name->text().toUtf8().toStdString()) : filters->currentText().toUtf8().toStdString();
  const auto quoted = Postprocessor::plot_quote(chosenName);
  if (quoted.isEmpty())
  {
    QMessageBox::warning(this,tr("Filter name"),tr("Enter a name that can be quoted in the journal."));
    return;
  }
  QString command = QString(create ? "post section create " : "post section plane ")+quoted;
  if (create)
  {
    const auto source = app->session().sources.find(app->session().active_source_id);
    if (source == app->session().sources.end())
    {
      return;
    }
    const auto sourceName = Postprocessor::plot_quote(source->second.name);
    if (sourceName.isEmpty())
    {
      return;
    }
    command += " source "+sourceName;
  }
  command += " "+Postprocessor::plane_command_text(plane);

  send(command);
}
void PostprocessorSectionControls::refresh()
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
  loading = true;
  auto selected = filters->currentData().toString();
  const auto active = app->session().representations.find(app->session().active_representation_id);
  if (active != app->session().representations.end() && active->second.input.owner == Postprocessor::OutputOwner::Feature && lastActive != app->session().active_representation_id)
  {
    const auto filter = app->session().filters.find(active->second.input.object_id);
    if (filter != app->session().filters.end() && filter->second.kind == Postprocessor::FilterKind::Section)
    {
      selected = QString::fromStdString(filter->first);
    }
  }
  lastActive = app->session().active_representation_id;
  filters->clear();
  filters->addItem(tr("New section..."),QString());
  for (const auto& entry : app->session().filters)
  {
    if (entry.second.kind == Postprocessor::FilterKind::Section)
    {
      filters->addItem(QString::fromUtf8(entry.second.name.c_str()),QString::fromStdString(entry.first));
    }
  }
  const int index = filters->findData(selected);
  filters->setCurrentIndex(index >= 0 ? index : 0);
  const auto found = app->session().filters.find(filters->currentData().toString().toStdString());
  visible->setEnabled(found != app->session().filters.end());
  if (found != app->session().filters.end())
  {
    name->setText(QString::fromUtf8(found->second.name.c_str()));
    for (int i = 0; i < 6; ++i)
    {
      if (!coordinates[i]->hasFocus())
      {
        coordinates[i]->setText(QString::number(i < 3 ? found->second.plane.origin[i] : found->second.plane.normal[i-3],'g',17));
      }
    }
    side->setCurrentIndex(found->second.keep == Postprocessor::ClipSide::Positive ? 0 : 1);
    bool shown = false;
    for (const auto& display : app->session().representations)
    {
      shown = shown || (display.second.input.owner == Postprocessor::OutputOwner::Feature && display.second.input.object_id == found->first &&
        display.second.view_id == app->session().active_view_id && display.second.visible);
    }
    visible->setChecked(shown);
  }
  else
  {
    name->setPlaceholderText(QString::fromStdString(app->suggested_filter_name(Postprocessor::FilterKind::Section)));
    visible->setChecked(false);
  }
  loading = false;
}
