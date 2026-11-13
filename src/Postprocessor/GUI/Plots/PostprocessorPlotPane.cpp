#include "PostprocessorPlotPane.hpp"
#include <utility>
#include "PostprocessorPlotChart.hpp"
#include "PostprocessorPlotRendering.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QToolButton>
#include <QLabel>
#include <QStyle>
#include <QMenu>
#include <QInputDialog>
#include <QLineEdit>
#include <QCursor>
#include <QIcon>
#include <QFileDialog>
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QSignalBlocker>
#include <QScopedValueRollback>
#include <algorithm>
#include <cmath>
PostprocessorPlotPane::PostprocessorPlotPane(std::weak_ptr<PostprocessorApplicationInterface> active,
  Dispatch dispatch,Dispatch silent,QWidget* parent)
  : QWidget(parent),application(std::move(active)),submitCommand(std::move(dispatch)),silentCommand(std::move(silent))
{
  setObjectName("Postprocessor.PlotPane");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  auto* header = new QHBoxLayout;
  selector = new QComboBox(this);
  selector->setObjectName("Postprocessor.PlotSelector");
  header->addWidget(selector,1);
  auto* options = new QToolButton(this);
  options->setText("…");
  options->setToolTip(tr("Plot commands"));
  header->addWidget(options);
  auto* close = new QToolButton(this);
  close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  close->setToolTip(tr("Close plot pane"));
  header->addWidget(close);
  layout->addLayout(header);
  status = new QLabel(this);
  status->setTextFormat(Qt::PlainText);
  status->setWordWrap(true);
  layout->addWidget(status);
  chart = new PostprocessorPlotChart([this](const Postprocessor::PlotAxes& axes,bool journal)
  {
    const auto app = application.lock();
    if (!app || !shown)
    {
      return false;
    }
    const auto plot = app->session().plots.find(shown->id);
    if (plot == app->session().plots.end())
    {
      return false;
    }
    std::string error;
    const auto command = Postprocessor::plot_axes_command(plot->second.name,axes);
    const bool accepted = journal ? submitCommand(command,error) : silentCommand(command,error);
    if (!accepted)
    {
      PRINT_ERROR("Cannot change plot axes: %s\n",error.c_str());
    }
    return accepted;
  },[this]()
  {
    const auto app = application.lock();
    return app ? app->session().plot_revision : std::uint64_t{0};
  },this);
  layout->addWidget(chart,1);
  connect(selector,&QComboBox::currentIndexChanged,this,[this](int index)
  {
    if (!synchronizing && index >= 0)
    {
      submit(QStringLiteral("post plot show ")+Postprocessor::plot_quote(selector->itemText(index).toUtf8().toStdString()));
    }
  });
  connect(options,&QToolButton::clicked,this,[this]()
  {
    commands_menu();
  });
  connect(close,&QToolButton::clicked,this,[this]()
  {
    submit("post plot close");
  });
}
void PostprocessorPlotPane::cancel()
{
  chart->cancel();
}
void PostprocessorPlotPane::submit(const QString& command)
{
  if (command.isEmpty())
  {
    PRINT_ERROR("The plot command or axis range is invalid.\n");
    return;
  }
  cancel();
  std::string error;
  if (!submitCommand(command,error))
  {
    PRINT_ERROR("Cannot submit plot command: %s\n",error.c_str());
  }
  synchronize();
}
void PostprocessorPlotPane::synchronize()
{
  const auto app = application.lock();
  if (!app || synchronizing)
  {
    return;
  }
  QScopedValueRollback<bool> guard(synchronizing,true);
  const auto& state = app->session();
  const auto view = state.views.find(state.active_view_id);
  const auto id = view != state.views.end() && view->second.plot_id ? *view->second.plot_id : std::string{};
  const auto found = state.plots.find(id);
  if (found == state.plots.end())
  {
    cancel();
    shown.reset();
    owners.clear();
    values.curves.clear();
    return;
  }
  auto definition = found->second;
  {
    const QSignalBlocker block(selector);
    selector->clear();
    for (const auto& entry : state.plots)
    {
      selector->addItem(QString::fromUtf8(entry.second.name.c_str()),QString::fromUtf8(entry.first.c_str()));
      if (entry.first == id)
      {
        selector->setCurrentIndex(selector->count()-1);
      }
    }
  }
  std::vector<std::shared_ptr<const SimResults::VtkResultsData>> handles;
  for (const auto& curve : definition.curves)
  {
    handles.push_back(app->result_data(Postprocessor::plot_curve_source(curve)));
  }
  bool extract = !shown || shown->id != id || shown->data_revision != definition.data_revision || handles != owners ||
    shown->curves.size() != definition.curves.size();
  if (!extract)
  {
    for (std::size_t index = 0; index < definition.curves.size(); ++index)
    {
      extract = extract || !Postprocessor::same_plot_input(shown->curves[index],definition.curves[index]) ||
        shown->curves[index].name != definition.curves[index].name;
    }
  }
  bool redraw = extract || !shown || shown->name != definition.name || shown->curves.size() != definition.curves.size();
  if (!redraw && shown)
  {
    for (std::size_t index = 0; index < definition.curves.size(); ++index)
    {
      redraw = redraw || shown->curves[index].visible != definition.curves[index].visible ||
        shown->curves[index].color != definition.curves[index].color;
    }
  }
  if (extract)
  {
    chart->cancel();
    definition = app->session().plots.at(id);
    std::string error;
    if (!app->plot_data(id,values,error))
    {
      values.curves.clear();
      status->setText(QString::fromUtf8(error.c_str()));
    }
    owners = std::move(handles);
  }
  QStringList messages;
  for (const auto& curve : values.curves)
  {
    if (!curve.status.empty())
    {
      messages << QString::fromUtf8((curve.name+": "+curve.status).c_str());
    }
  }
  if (!values.curves.empty() || definition.curves.empty())
  {
    status->setText(messages.join('\n'));
  }
  if (redraw)
  {
    chart->set_data(definition,values);
  }
  else if (!shown || shown->axes.x != definition.axes.x || shown->axes.y != definition.axes.y)
  {
    chart->apply_axes(definition.axes);
  }
  const auto marker = plot_frame_marker(state,definition,values);
  chart->set_marker(marker);
  shown = definition;
}
void PostprocessorPlotPane::commands_menu()
{
  using namespace Postprocessor;
  const auto app = application.lock();
  if (!app)
  {
    return;
  }
  QMenu menu(this);
  auto* create = menu.addAction(tr("New plot…"));
  connect(create,&QAction::triggered,&menu,[this]()
  {
    bool ok = false;
    const auto name = QInputDialog::getText(this,tr("New plot"),tr("Name"),QLineEdit::Normal,{},&ok);
    if (ok && !Postprocessor::plot_quote(name.toUtf8().toStdString()).isEmpty())
    {
      submit(QStringLiteral("post plot create ")+Postprocessor::plot_quote(name.toUtf8().toStdString()));
    }
  });
  if (shown)
  {
    const auto definition = *shown;
    const auto scope = QStringLiteral(" plot ")+plot_quote(definition.name);
    auto* fit = menu.addAction(QIcon(":/cubitgui/zoomreset.svg"),tr("Fit axes"));
    connect(fit,&QAction::triggered,&menu,[this,scope]()
    {
      submit("post plot axes auto"+scope);
    });
    auto* ranges = menu.addAction(tr("Axis ranges…"));
    connect(ranges,&QAction::triggered,this,[this]()
    {
      axis_dialog();
    });
    auto* rename = menu.addAction(tr("Rename plot…"));
    connect(rename,&QAction::triggered,&menu,[this,definition]()
    {
      bool ok = false;
      const auto name = QInputDialog::getText(this,tr("Rename plot"),tr("Name"),QLineEdit::Normal,
        QString::fromUtf8(definition.name.c_str()),&ok);
      if (ok && !Postprocessor::plot_quote(name.toUtf8().toStdString()).isEmpty())
      {
        submit(QStringLiteral("post plot rename ")+Postprocessor::plot_quote(definition.name)+" "+Postprocessor::plot_quote(name.toUtf8().toStdString()));
      }
    });
    auto* curves = menu.addMenu(tr("Curves"));
    for (const auto& curve : definition.curves)
    {
      auto* entry = curves->addMenu(QString::fromUtf8(curve.name.c_str()));
      auto* visible = entry->addAction(tr("Visible"));
      visible->setCheckable(true);
      visible->setChecked(curve.visible);
      connect(visible,&QAction::triggered,&menu,[this,scope,curve](bool checked)
      {
        submit(QStringLiteral("post plot curve ")+(checked ? "show " : "hide ")+Postprocessor::plot_quote(curve.name)+scope);
      });
      auto* remove = entry->addAction(tr("Remove curve"));
      connect(remove,&QAction::triggered,&menu,[this,scope,curve]()
      {
        submit(QStringLiteral("post plot curve remove ")+Postprocessor::plot_quote(curve.name)+scope);
      });
    }
    auto* csv = menu.addAction(tr("Export CSV…"));
    connect(csv,&QAction::triggered,&menu,[this,scope]()
    {
      const auto path = QFileDialog::getSaveFileName(this,tr("Export plot data"),{},"CSV (*.csv)",nullptr,QFileDialog::DontConfirmOverwrite);
      if (!path.isEmpty())
      {
        submit(QStringLiteral("post plot export csv ")+Postprocessor::plot_quote(path.toUtf8().toStdString())+scope);
      }
    });
    auto* remove = menu.addAction(tr("Remove plot"));
    connect(remove,&QAction::triggered,&menu,[this,definition]()
    {
      submit(QStringLiteral("post plot remove ")+Postprocessor::plot_quote(definition.name));
    });
  }
  menu.exec(QCursor::pos());
}
void PostprocessorPlotPane::axis_dialog()
{
  if (!shown)
  {
    return;
  }
  const auto name = shown->name;
  const auto axes = chart->current_axes();
  QDialog dialog(this);
  dialog.setWindowTitle(tr("Plot axes"));
  auto* form = new QFormLayout(&dialog);
  auto* automatic = new QCheckBox(tr("Automatic ranges"),&dialog);
  automatic->setChecked(!shown->axes.x && !shown->axes.y);
  form->addRow(automatic);
  QDoubleSpinBox* fields[4];
  const QString labels[] = {tr("X minimum"),tr("X maximum"),tr("Y minimum"),tr("Y maximum")};
  for (int index = 0; index < 4; ++index)
  {
    fields[index] = new QDoubleSpinBox(&dialog);
    fields[index]->setDecimals(12);
    fields[index]->setRange(-1e100,1e100);
    fields[index]->setValue(index < 2 ? (*axes.x)[index] : (*axes.y)[index-2]);
    fields[index]->setEnabled(!automatic->isChecked());
    connect(automatic,&QCheckBox::toggled,fields[index],[field = fields[index]](bool checked)
    {
      field->setEnabled(!checked);
    });
    form->addRow(labels[index],fields[index]);
  }
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
  connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
  connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
  form->addRow(buttons);
  if (dialog.exec() == QDialog::Accepted)
  {
    Postprocessor::PlotAxes result;
    if (!automatic->isChecked())
    {
      result.x = std::array<double,2>{fields[0]->value(),fields[1]->value()};
      result.y = std::array<double,2>{fields[2]->value(),fields[3]->value()};
    }
    submit(Postprocessor::plot_axes_command(name,result));
  }
}

QSize PostprocessorPlotPane::chart_size() const
{
  return chart->size();
}
