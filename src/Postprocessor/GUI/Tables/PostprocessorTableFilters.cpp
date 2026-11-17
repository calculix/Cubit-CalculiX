#include "PostprocessorTableFilters.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include <QComboBox>
#include <QCompleter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <algorithm>
#include <cmath>
#include <set>
PostprocessorTableFilterProxy::PostprocessorTableFilterProxy(QObject* parent) : QSortFilterProxyModel(parent)
{
}
void PostprocessorTableFilterProxy::set_table(const Postprocessor::ResultTableData& data)
{
  values = data;
  prepared = Postprocessor::prepare_table_filters(values,values.filters);
  invalidateFilter();
  if (!data.columns.empty())
  {
    emit headerDataChanged(Qt::Horizontal,0,static_cast<int>(data.columns.size())-1);
  }
}
bool PostprocessorTableFilterProxy::filterAcceptsRow(int row,const QModelIndex&) const
{
  return row >= 0 && Postprocessor::prepared_table_row_matches(values,static_cast<std::size_t>(row),prepared);
}
QVariant PostprocessorTableFilterProxy::headerData(int section,Qt::Orientation orientation,int role) const
{
  auto result = QSortFilterProxyModel::headerData(section,orientation,role);
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 &&
      static_cast<std::size_t>(section) < values.columns.size())
  {
    for (const auto& filter : values.filters)
    {
      if (filter.column == values.columns[static_cast<std::size_t>(section)])
      {
        return result.toString()+QStringLiteral(" [filtered]");
      }
    }
  }
  return result;
}
QString table_filter_clause(const Postprocessor::TableFilter& filter)
{
  const auto column = Postprocessor::plot_quote(filter.column);
  if (column.isEmpty())
  {
    return {};
  }
  if (filter.mode == Postprocessor::TableFilterMode::Range)
  {
    return column+" range "+QString::number(filter.minimum,'g',17)+" "+QString::number(filter.maximum,'g',17);
  }
  QString value = QStringLiteral("\"\"");
  if (!filter.value.empty())
  {
    value = Postprocessor::plot_quote(filter.value);
    if (value.isEmpty())
    {
      return {};
    }
  }
  return column+" equals "+value;
}
bool edit_table_column_filter(QWidget* parent,const Postprocessor::ResultTableData& data,std::size_t column,
  Postprocessor::TableFilter& filter)
{
  if (column >= data.columns.size())
  {
    return false;
  }
  QDialog dialog(parent);
  dialog.setWindowTitle(QObject::tr("Column filter"));
  auto* form = new QFormLayout(&dialog);
  auto* heading = new QLabel(QString::fromStdString(data.columns[column]),&dialog);
  heading->setTextFormat(Qt::PlainText);
  form->addRow(QObject::tr("Column"),heading);
  auto* mode = new QComboBox(&dialog);
  mode->addItem(QObject::tr("Exact value"));
  if (Postprocessor::table_filter_range_supported(data,column))
  {
    mode->addItem(QObject::tr("Inclusive range"));
  }
  auto* value = new QComboBox(&dialog);
  value->setObjectName("Postprocessor.Filter.Value");
  value->setEditable(true);
  value->setInsertPolicy(QComboBox::NoInsert);
  auto suggestionData = data;
  if (!suggestionData.presentation_units)
  {
    SimUnits::WorkspaceUnitContext context;
    std::string error;
    if (framework_units().workspace_context("postprocessor",context,error))
    {
      suggestionData.presentation_units = context;
    }
  }
  if (!filter.unit.empty() && column < data.column_units.size())
  {
    if (!suggestionData.presentation_units)
    {
      suggestionData.presentation_units = SimUnits::WorkspaceUnitContext("postprocessor");
    }
    suggestionData.presentation_units->display_units[data.column_units[column].quantity] = filter.unit;
  }
  std::set<std::string> distinct;
  for (std::size_t row = 0; row < data.rows && distinct.size() <= 10000; ++row)
  {
    distinct.insert(Postprocessor::result_table_cell(suggestionData,row,column));
  }
  for (const auto& entry : distinct)
  {
    value->addItem(QString::fromStdString(entry));
  }
  value->completer()->setFilterMode(Qt::MatchContains);
  value->completer()->setCaseSensitivity(Qt::CaseSensitive);
  value->completer()->setCompletionMode(QCompleter::PopupCompletion);
  auto* minimum = new QLineEdit(&dialog);
  auto* maximum = new QLineEdit(&dialog);
  auto* unit = new QLabel(QString::fromStdString(Postprocessor::result_table_column_unit(suggestionData,column)),&dialog);
  unit->setTextFormat(Qt::PlainText);
  auto* message = new QLabel(&dialog);
  message->setTextFormat(Qt::PlainText);
  message->setWordWrap(true);
  form->addRow(QObject::tr("Mode"),mode);
  form->addRow(QObject::tr("Value (search or type)"),value);
  form->addRow(QObject::tr("Minimum"),minimum);
  form->addRow(QObject::tr("Maximum"),maximum);
  form->addRow(QObject::tr("Unit"),unit);
  if (distinct.size() > 10000)
  {
    auto* notice = new QLabel(QObject::tr("Suggestions limited to the first 10001 distinct values. Any exact value can be typed."),&dialog);
    notice->setWordWrap(true);
    form->addRow(notice);
  }
  form->addRow(message);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
  form->addRow(buttons);
  Postprocessor::TableFilter draft = filter;
  draft.column = data.columns[column];
  if (draft.unit.empty() && column < data.column_units.size() && !data.column_units[column].unit.empty())
  {
    draft.unit = Postprocessor::result_table_column_unit(suggestionData,column);
  }
  // Existing values use their frozen unit, independent of current display units.
  if (!draft.unit.empty())
  {
    unit->setText(QString::fromStdString(draft.unit));
  }
  if (draft.mode == Postprocessor::TableFilterMode::Range)
  {
    mode->setCurrentIndex(1);
  }
  value->setEditText(QString::fromStdString(draft.value));
  minimum->setText(QString::number(draft.minimum,'g',17));
  maximum->setText(QString::number(draft.maximum,'g',17));
  const auto read = [&]()
  {
    draft.mode = Postprocessor::TableFilterMode::Equals;
    draft.value = value->currentText().toStdString();
    if (mode->currentIndex() == 1)
    {
      draft.mode = Postprocessor::TableFilterMode::Range;
      bool low = false,high = false;
      draft.minimum = minimum->text().toDouble(&low);
      draft.maximum = maximum->text().toDouble(&high);
      if (!low || !high || !std::isfinite(draft.minimum) || !std::isfinite(draft.maximum))
      {
        return false;
      }
    }
    std::vector<Postprocessor::TableFilter> filters{draft};
    std::string error;
    if (!Postprocessor::validate_table_filters(data,filters,error) || table_filter_clause(draft).isEmpty())
    {
      message->setText(QString::fromStdString(error));
      return false;
    }
    draft = filters.front();
    return true;
  };
  QTimer preview;
  preview.setSingleShot(true);
  preview.setInterval(250);
  const auto changed = [&]()
  {
    const bool range = mode->currentIndex() == 1;
    value->setEnabled(!range);
    minimum->setEnabled(range);
    maximum->setEnabled(range);
    preview.start();
  };
  QObject::connect(&preview,&QTimer::timeout,&dialog,[&]()
  {
    const bool valid = read();
    buttons->button(QDialogButtonBox::Ok)->setEnabled(valid);
    if (valid)
    {
      const auto prepared = Postprocessor::prepare_table_filters(data,{draft});
      std::size_t count = 0;
      for (std::size_t row = 0; row < data.rows; ++row)
      {
        if (Postprocessor::prepared_table_row_matches(data,row,prepared))
        {
          ++count;
        }
      }
      message->setText(QObject::tr("%1 of %2 rows match this column filter.").arg(static_cast<qulonglong>(count)).arg(static_cast<qulonglong>(data.rows)));
    }
  });
  QObject::connect(mode,&QComboBox::currentIndexChanged,&dialog,[&](int)
  {
    changed();
  });
  QObject::connect(value,&QComboBox::editTextChanged,&dialog,[&](const QString&)
  {
    changed();
  });
  QObject::connect(minimum,&QLineEdit::textChanged,&dialog,[&](const QString&)
  {
    changed();
  });
  QObject::connect(maximum,&QLineEdit::textChanged,&dialog,[&](const QString&)
  {
    changed();
  });
  QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]()
  {
    if (read())
    {
      dialog.accept();
    }
  });
  QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
  changed();
  if (dialog.exec() != QDialog::Accepted)
  {
    return false;
  }
  filter = draft;
  return true;
}
