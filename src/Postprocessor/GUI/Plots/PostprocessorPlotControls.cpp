#include <type_traits>
#include "PostprocessorPlotControls.hpp"
#include "Postprocessor/Commands/tables/TableCommandText.hpp"
#include <QMessageBox>
#include <utility>
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QMenu>
#include <algorithm>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QComboBox>
#include <QLineEdit>
void add_plot_context_actions(QMenu& menu,const PostprocessorApplicationInterface& application,
  const Postprocessor::EntityReference& entity,const std::function<void(const QString&)>& submit)
{
  using namespace Postprocessor;
  const auto add = [&](const QString& title,PlotReduction reduction)
  {
    std::string error;
    const auto commands = plot_entity_commands(application,entity,reduction,&error);
    auto* action = menu.addAction(title);
    action->setEnabled(!commands.empty());
    action->setToolTip(QString::fromUtf8(error.c_str()));
    QObject::connect(action,&QAction::triggered,&menu,[commands,submit]()
    {
      for (const auto& command : commands)
      {
        submit(command);
      }
    });
  };
  if (entity.kind == EntityKind::Element)
  {
    add(QObject::tr("Plot history — element value"),PlotReduction::None);
    add(QObject::tr("Plot history — nodal minimum"),PlotReduction::Minimum);
    add(QObject::tr("Plot history — nodal maximum"),PlotReduction::Maximum);
  }
  else
  {
    add(QObject::tr("Plot over all frames"),PlotReduction::None);
  }
}
void plot_table_dialog(QWidget* parent,const PostprocessorApplicationInterface& application,
  const Postprocessor::TableReference& reference,const Postprocessor::ResultTableData& data,
  std::size_t row,std::size_t column,const std::function<void(const QString&)>& submit)
{
  using namespace Postprocessor;
  const auto source = application.session().sources.find(reference.source_id);
  if (source == application.session().sources.end())
  {
    return;
  }
  const auto sourceName = source->second.name;
  auto plots = application.session().plots;
  const auto activePlot = application.session().active_plot_id;
  QDialog dialog(parent);
  dialog.setWindowTitle(QObject::tr("Plot table columns"));
  auto* form = new QFormLayout(&dialog);
  auto* x = new QComboBox(&dialog);
  auto* y = new QComboBox(&dialog);
  auto* filter = new QComboBox(&dialog);
  auto* value = new QLineEdit(&dialog);
  auto* plot = new QComboBox(&dialog);
  filter->addItem(QObject::tr("None"),-1);
  for (std::size_t index = 0; index < data.columns.size(); ++index)
  {
    const auto name = QString::fromUtf8(data.columns[index].c_str());
    if (index < data.numeric.size() && data.numeric[index])
    {
      x->addItem(name,static_cast<int>(index));
      y->addItem(name,static_cast<int>(index));
    }
    filter->addItem(name,static_cast<int>(index));
  }
  const int time = x->findText("Time");
  x->setCurrentIndex([&]() -> std::common_type_t<decltype(time), decltype(0)> {
    if (time >= 0) {
      return time;
    } else {
      return 0;
    }
  }());
  const int selected = y->findData(static_cast<int>(column));
  y->setCurrentIndex([&]() -> std::common_type_t<decltype(selected), decltype(0)> {
    if (selected >= 0) {
      return selected;
    } else {
      return 0;
    }
  }());
  plot->addItem(QObject::tr("New plot"),QString());
  for (const auto& entry : plots)
  {
    plot->addItem(QString::fromUtf8(entry.second.name.c_str()),QString::fromUtf8(entry.first.c_str()));
    if (entry.first == activePlot)
    {
      plot->setCurrentIndex(plot->count()-1);
    }
  }
  QObject::connect(filter,&QComboBox::currentIndexChanged,&dialog,[&](int)
  {
    const int index = filter->currentData().toInt();
    value->setEnabled(index >= 0);
    if (index >= 0 && row < data.rows)
    {
      value->setText(QString::fromUtf8(result_table_cell(data,row,static_cast<std::size_t>(index)).c_str()));
    }
  });
  value->setEnabled(false);
  form->addRow(QObject::tr("X column"),x);
  form->addRow(QObject::tr("Y column"),y);
  form->addRow(QObject::tr("Filter column"),filter);
  form->addRow(QObject::tr("Exact value"),value);
  form->addRow(QObject::tr("Plot"),plot);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
  QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
  QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
  form->addRow(buttons);
  if (dialog.exec() != QDialog::Accepted || x->currentIndex() < 0 || y->currentIndex() < 0)
  {
    return;
  }
  std::string error;
  const auto selector = table_selector_text(application,reference,error);
  const auto sourceArg = plot_quote(sourceName);
  const auto xArg = plot_quote(x->currentText().toUtf8().toStdString());
  const auto yArg = plot_quote(y->currentText().toUtf8().toStdString());
  const auto filterArg = [&]() -> std::common_type_t<decltype(plot_quote(filter->currentText().toUtf8().toStdString())), decltype(QString())> {
    if (filter->currentIndex() > 0) {
      return plot_quote(filter->currentText().toUtf8().toStdString());
    } else {
      return QString();
    }
  }();
  const auto valueArg = [&]() -> std::common_type_t<decltype(QStringLiteral("\"\"")), decltype(plot_quote(value->text().toUtf8().toStdString()))> {
    if (value->text().isEmpty()) {
      return QStringLiteral("\"\"");
    } else {
      return plot_quote(value->text().toUtf8().toStdString());
    }
  }();
  if (selector.isEmpty() || sourceArg.isEmpty() || xArg.isEmpty() || yArg.isEmpty() ||
      (filter->currentIndex() > 0 && (filterArg.isEmpty() || valueArg.isEmpty())) ||
      application.result_data(reference.source_id) != data.owner)
  {
    QMessageBox::warning(parent,QObject::tr("Plot table columns"),[&]() -> std::common_type_t<decltype(QObject::tr("The table changed or its command arguments cannot be quoted. Reopen it before plotting.")), decltype(QString::fromUtf8(error.c_str()))> {
      if (error.empty()) {
        return QObject::tr("The table changed or its command arguments cannot be quoted. Reopen it before plotting.");
      } else {
        return QString::fromUtf8(error.c_str());
      }
    }());
    return;
  }
  std::string plotName;
  const auto id = plot->currentData().toString().toStdString();
  if (!id.empty())
  {
    plotName = plots.at(id).name;
  }
  else
  {
    std::size_t number = 1;
    bool found = true;
    while (found)
    {
      plotName = "Table history "+std::to_string(number++);
      found = false;
      for (const auto& entry : plots)
      {
        found = found || entry.second.name == plotName;
      }
    }
    submit(QStringLiteral("post plot create ")+plot_quote(plotName));
  }
  auto command = QString([&]() -> const char* {
    if (reference.field) {
      return "post plot add field ";
    } else {
      return "post plot add table ";
    }
  }())+selector+
    " x "+xArg+" y "+yArg;
  if (filter->currentIndex() > 0)
  {
    command += " where "+filterArg+" equals "+valueArg;
  }
  std::size_t number = 1;
  const auto selectedPlot = plots.find(id);
  while (selectedPlot != plots.end() && std::any_of(selectedPlot->second.curves.begin(),selectedPlot->second.curves.end(),[&](const auto& curve)
    {
      return curve.name == "curve_"+std::to_string(number);
    }))
  {
    ++number;
  }
  command += " source "+sourceArg+" name "+plot_quote("curve_"+std::to_string(number))+" plot "+plot_quote(plotName);
  submit(command);
}
