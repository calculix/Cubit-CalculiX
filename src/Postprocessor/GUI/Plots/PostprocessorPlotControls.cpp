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
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>
#include "Postprocessor/GUI/Tables/PostprocessorTableFilters.hpp"
#include "PostprocessorPlotLabels.hpp"
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
  const auto plots = application.session().plots;
  const auto activePlot = application.session().active_plot_id;
  const auto activeView = application.session().active_view_id;
  QDialog dialog(parent);
  dialog.setWindowTitle(QObject::tr("Plot table columns"));
  auto* form = new QFormLayout(&dialog);
  auto* x = new QComboBox(&dialog);
  auto* y = new QComboBox(&dialog);
  auto* filterColumn = new QComboBox(&dialog);
  auto* plot = new QComboBox(&dialog);
  for (std::size_t index = 0; index < data.columns.size(); ++index)
  {
    const auto name = QString::fromStdString(data.columns[index]);
    if (index < data.numeric.size() && data.numeric[index])
    {
      x->addItem(name,static_cast<int>(index));
      y->addItem(name,static_cast<int>(index));
    }
    filterColumn->addItem(name,static_cast<int>(index));
  }
  const int time = x->findText("Time");
  if (time >= 0)
  {
    x->setCurrentIndex(time);
  }
  int selected = y->findData(static_cast<int>(column));
  const auto measurement = [](const QString& name)
  {
    return name != "Frame" && name != "Step" && name != "Increment" && name != "Time" && name != "RecordIndex" &&
      name != "Valid" && name != "Dataset" && !name.endsWith("Id") && !name.endsWith("ID") && name != "IP";
  };
  if (selected < 0 || !measurement(y->itemText(selected)))
  {
    for (int index = 0; index < y->count(); ++index)
    {
      if (measurement(y->itemText(index)))
      {
        selected = index;
        break;
      }
    }
  }
  y->setCurrentIndex(selected);
  plot->addItem(QObject::tr("New plot"),QString());
  for (const auto& entry : plots)
  {
    plot->addItem(QString::fromStdString(entry.second.name),QString::fromStdString(entry.first));
    if (entry.first == activePlot)
    {
      plot->setCurrentIndex(plot->count()-1);
    }
  }
  form->addRow(QObject::tr("X column"),x);
  form->addRow(QObject::tr("Y column"),y);
  auto* filtersList = new QListWidget(&dialog);
  filtersList->setObjectName("Postprocessor.Plot.Filters");
  auto* countLabel = new QLabel(&dialog);
  countLabel->setTextFormat(Qt::PlainText);
  std::vector<TableFilter> filters;
  const auto updateList = [&]()
  {
    filtersList->clear();
    for (const auto& filter : filters)
    {
      filtersList->addItem(table_filter_clause(filter));
    }
    const auto prepared = prepare_table_filters(data,filters);
    std::size_t count = 0;
    for (std::size_t index = 0; index < data.rows; ++index)
    {
      if (prepared_table_row_matches(data,index,prepared))
      {
        ++count;
      }
    }
    countLabel->setText(QObject::tr("%1 of %2 rows match.").arg(static_cast<qulonglong>(count)).arg(static_cast<qulonglong>(data.rows)));
  };
  auto* import = new QPushButton(QObject::tr("Use table filters"),&dialog);
  import->setEnabled(!reference.filters.empty());
  QObject::connect(import,&QPushButton::clicked,&dialog,[&]()
  {
    filters = reference.filters;
    updateList();
  });
  auto* add = new QPushButton(QObject::tr("Add / edit filter…"),&dialog);
  auto* remove = new QPushButton(QObject::tr("Remove selected filter"),&dialog);
  QObject::connect(add,&QPushButton::clicked,&dialog,[&]()
  {
    const int index = filterColumn->currentData().toInt();
    if (index < 0 || static_cast<std::size_t>(index) >= data.columns.size())
    {
      return;
    }
    TableFilter filter;
    filter.column = data.columns[static_cast<std::size_t>(index)];
    if (row < data.rows)
    {
      filter.value = result_table_cell(data,row,static_cast<std::size_t>(index));
    }
    for (const auto& existing : filters)
    {
      if (existing.column == filter.column)
      {
        filter = existing;
      }
    }
    if (edit_table_column_filter(&dialog,data,static_cast<std::size_t>(index),filter))
    {
      filters.erase(std::remove_if(filters.begin(),filters.end(),[&](const auto& existing)
      {
        return existing.column == filter.column;
      }),filters.end());
      filters.push_back(filter);
      updateList();
    }
  });
  QObject::connect(remove,&QPushButton::clicked,&dialog,[&]()
  {
    const int index = filtersList->currentRow();
    if (index >= 0 && static_cast<std::size_t>(index) < filters.size())
    {
      filters.erase(filters.begin()+index);
      updateList();
    }
  });
  QObject::connect(filtersList,&QListWidget::currentRowChanged,&dialog,[&](int index)
  {
    if (index >= 0 && static_cast<std::size_t>(index) < filters.size())
    {
      filterColumn->setCurrentText(QString::fromStdString(filters[static_cast<std::size_t>(index)].column));
    }
  });
  form->addRow(QObject::tr("Filter column"),filterColumn);
  form->addRow(add);
  form->addRow(QObject::tr("Filters (AND)"),filtersList);
  form->addRow(countLabel);
  updateList();
  form->addRow(remove);
  form->addRow(import);
  form->addRow(QObject::tr("Plot"),plot);
  auto* title = new QLineEdit(&dialog);
  auto* xlabel = new QLineEdit(&dialog);
  auto* ylabel = new QLineEdit(&dialog);
  auto* caption = new QLineEdit(&dialog);
  for (auto* input : {title,xlabel,ylabel,caption})
  {
    input->setPlaceholderText(QObject::tr("Automatic / keep existing"));
    input->setMaxLength(256);
  }
  form->addRow(QObject::tr("Title"),title);
  form->addRow(QObject::tr("X axis label"),xlabel);
  form->addRow(QObject::tr("Y axis label"),ylabel);
  form->addRow(QObject::tr("Curve caption"),caption);
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
  if (selector.isEmpty() || application.result_data(reference.source_id) != data.owner || application.session().active_view_id != activeView ||
      !validate_table_filters(data,filters,error))
  {
    QMessageBox::warning(parent,QObject::tr("Plot table columns"),QObject::tr("The table changed or a filter is invalid. Reopen the dialog.")+QString::fromStdString(error));
    return;
  }
  for (auto* input : {title,xlabel,ylabel,caption})
  {
    if (!input->text().isEmpty() && !valid_plot_name(input->text().toStdString()))
    {
      QMessageBox::warning(parent,QObject::tr("Plot labels"),QObject::tr("Labels must be journalable text of at most 256 bytes."));
      return;
    }
  }
  std::string plotName;
  const auto id = plot->currentData().toString().toStdString();
  if (!id.empty())
  {
    const auto current = application.session().plots.find(id);
    if (current == application.session().plots.end() || current->second.name != plots.at(id).name)
    {
      return;
    }
    plotName = current->second.name;
  }
  else
  {
    std::size_t number = 1;
    bool found = true;
    while (found)
    {
      plotName = "Table history "+std::to_string(number++);
      found = std::any_of(application.session().plots.begin(),application.session().plots.end(),[&](const auto& entry)
      {
        return entry.second.name == plotName;
      });
    }
  }
  std::size_t number = 1;
  const auto selectedPlot = application.session().plots.find(id);
  while (selectedPlot != application.session().plots.end() && std::any_of(selectedPlot->second.curves.begin(),selectedPlot->second.curves.end(),[&](const auto& curve)
    {
      return curve.name == "curve_"+std::to_string(number);
    }))
  {
    ++number;
  }
  const auto curveName = "curve_"+std::to_string(number);
  const auto xArgument = plot_quote(x->currentText().toStdString());
  const auto yArgument = plot_quote(y->currentText().toStdString());
  const auto sourceArgument = plot_quote(sourceName);
  const auto plotArgument = plot_quote(plotName);
  if (xArgument.isEmpty() || yArgument.isEmpty() || sourceArgument.isEmpty() || plotArgument.isEmpty())
  {
    QMessageBox::warning(parent,QObject::tr("Plot table columns"),QObject::tr("The selected names cannot be represented in a journal command."));
    return;
  }
  QString command = "post plot add table ";
  if (reference.field)
  {
    command = "post plot add field ";
  }
  command += selector+" x "+xArgument+" y "+yArgument;
  for (const auto& filter : filters)
  {
    const auto clause = table_filter_clause(filter);
    if (clause.isEmpty())
    {
      return;
    }
    QString unit = QStringLiteral("\"\"");
    if (!filter.unit.empty())
    {
      unit = plot_quote(filter.unit);
    }
    command += " where "+clause+" unit "+unit;
  }
  command += " source "+plot_quote(sourceName)+" name "+plot_quote(curveName)+" plot "+plot_quote(plotName);
  // Validate the full history before creating a plot, avoiding empty plots on failure.
  PlotTableReference requested{reference.source_id,reference.key,x->currentText().toStdString(),y->currentText().toStdString(),"","",reference.field};
  requested.filters = filters;
  PlotCurveData preview;
  if (!plot_table_data(data.owner,requested,preview,error))
  {
    QMessageBox::warning(parent,QObject::tr("Plot table columns"),QString::fromStdString(error));
    return;
  }
  if (id.empty())
  {
    submit("post plot create "+plot_quote(plotName));
  }
  submit(command);
  const auto committed = std::find_if(application.session().plots.begin(),application.session().plots.end(),[&](const auto& entry)
  {
    return entry.second.name == plotName && std::any_of(entry.second.curves.begin(),entry.second.curves.end(),[&](const auto& curve)
    {
      return curve.name == curveName;
    });
  });
  if (committed == application.session().plots.end())
  {
    return;
  }
  const auto label = [&](const char* target,QLineEdit* input)
  {
    if (!input->text().isEmpty())
    {
      submit(plot_label_command(plotName,target,input->text().toStdString(),curveName));
    }
  };
  label("title",title);
  label("xlabel",xlabel);
  label("ylabel",ylabel);
  label("curve",caption);
}
