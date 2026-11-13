#include "PostprocessorTableView.hpp"
#include "PostprocessorTableModel.hpp"
#include <QLabel>
#include <QMenu>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QHeaderView>
#include <QScrollBar>
#include <QFontMetrics>
#include <QStyle>
#include <QTimer>
#include <QShowEvent>
#include <QResizeEvent>
#include <QScopedValueRollback>
#include <algorithm>
#include <utility>
PostprocessorTableView::PostprocessorTableView(std::function<void()> closeTable,QWidget* parent,Plot plot) : QWidget(parent)
{
  setObjectName("Postprocessor.TablePane");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  title = new QLabel(this);
  title->setTextFormat(Qt::PlainText);
  title->setTextInteractionFlags(Qt::TextSelectableByMouse);
  auto* heading = new QHBoxLayout();
  heading->addWidget(title,1);
  auto* close = new QToolButton(this);
  close->setObjectName("Postprocessor.CloseTable");
  close->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));
  close->setToolTip(tr("Close table view"));
  connect(close,&QToolButton::clicked,this,[closeTable]()
  {
    if (closeTable)
    {
      closeTable();
    }
  });
  heading->addWidget(close);
  layout->addLayout(heading);
  model = new PostprocessorTableModel(this);
  auto* sorted = new QSortFilterProxyModel(this);
  sorted->setSourceModel(model);
  sorted->setSortRole(Qt::UserRole);
  table = new QTableView(this);
  table->setObjectName("Postprocessor.TableView");
  table->setModel(sorted);
  // Header sorting awaits its command contract; retain source row order.
  table->setSortingEnabled(false);
  table->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(table,&QTableView::customContextMenuRequested,this,[this,plot,sorted](const QPoint& point)
  {
    const auto index = sorted->mapToSource(table->indexAt(point));
    if (!plot || !plotReference || !index.isValid())
    {
      return;
    }
    const auto reference = *plotReference;
    const auto data = shown;
    QMenu menu(this);
    auto* action = menu.addAction(tr("Plot columns…"));
    connect(action,&QAction::triggered,&menu,[plot,reference,data,index]()
    {
      plot(reference,data,static_cast<std::size_t>(index.row()),static_cast<std::size_t>(index.column()));
    });
    menu.exec(table->viewport()->mapToGlobal(point));
  });
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  table->setWordWrap(false);
  table->setTextElideMode(Qt::ElideNone);
  table->horizontalHeader()->setStretchLastSection(false);
  table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  connect(table->horizontalHeader(),&QHeaderView::sectionResized,this,[this](int column,int,int)
  {
    if (!resizingColumns)
    {
      manualColumns.insert(column);
    }
  });
  connect(table->verticalScrollBar(),&QScrollBar::valueChanged,this,[this](int)
  {
    queue_column_fit();
  });
  layout->addWidget(table,1);
}
void PostprocessorTableView::set_plot_reference(const std::optional<Postprocessor::TableReference>& reference)
{
  plotReference = reference;
}
void PostprocessorTableView::show_data(Postprocessor::ResultTableData data)
{
  title->setText(QString::fromUtf8((data.title+(data.status.empty() ? "" : " — "+data.status)).c_str()));
  const bool newTable = shown.owner != data.owner || shown.title != data.title || shown.columns != data.columns || shown.frame_columns != data.frame_columns;
  bool same = !newTable && shown.table == data.table && shown.rows == data.rows && shown.parts.size() == data.parts.size();
  for (std::size_t index = 0; same && index < data.parts.size(); ++index)
  {
    const auto& old = shown.parts[index];
    const auto& current = data.parts[index];
    same = old.mesh == current.mesh && old.dataset == current.dataset && old.first_row == current.first_row &&
      old.count == current.count && old.frame == current.frame && old.table == current.table &&
      old.column_map == current.column_map && old.field.array_name == current.field.array_name && old.field.association == current.field.association;
  }
  if (same)
  {
    shown.active_frame = data.active_frame;
    shown.status = data.status;
    model->set_active_frame(data.active_frame);
    return;
  }
  shown = data; // Copies metadata and owning handles; never materializes result rows.
  model->set_data(std::move(data));
  if (newTable)
  {
    manualColumns.clear();
    resize_columns_to_contents();
  }
  else
  {
    widen_columns_for_visible_rows();
  }
  queue_column_fit();
}
void PostprocessorTableView::measure_rows(const std::vector<int>& rows,bool reset)
{
  const QScopedValueRollback<bool> guard(resizingColumns,true);
  const QFontMetrics cells(table->font());
  const QFontMetrics headings(table->horizontalHeader()->font());
  const int padding = 2*style()->pixelMetric(QStyle::PM_HeaderMargin)+16;
  for (int column = 0; column < table->model()->columnCount(); ++column)
  {
    if (!reset && manualColumns.contains(column))
    {
      continue;
    }
    int width = headings.horizontalAdvance(table->model()->headerData(column,Qt::Horizontal).toString())+padding;
    for (int row : rows)
    {
      width = std::max(width,cells.horizontalAdvance(table->model()->index(row,column).data(Qt::DisplayRole).toString())+padding);
    }
    if (reset || width > table->columnWidth(column))
    {
      table->setColumnWidth(column,width);
    }
  }
}
void PostprocessorTableView::resize_columns_to_contents()
{
  std::vector<int> rows;
  const int count = table->model()->rowCount();
  const int samples = std::min(count,Postprocessor::TableDefaults::sizing_sample_rows);
  for (int index = 0; index < samples; ++index)
  {
    rows.push_back(samples <= 1 ? 0 : static_cast<int>(static_cast<qint64>(index)*(count-1)/(samples-1)));
  }
  measure_rows(rows,true);
}
void PostprocessorTableView::widen_columns_for_visible_rows()
{
  const int count = table->model()->rowCount();
  if (!count)
  {
    return;
  }
  const int first = std::max(0,table->rowAt(0));
  int last = table->rowAt(table->viewport()->height()-1);
  if (last < first)
  {
    last = std::min(count-1,first+std::max(1,table->viewport()->height()/std::max(1,table->verticalHeader()->defaultSectionSize())));
  }
  std::vector<int> rows;
  for (int row = first; row <= last && row < count; ++row)
  {
    rows.push_back(row);
  }
  measure_rows(rows,false);
}
void PostprocessorTableView::queue_column_fit()
{
  if (fitQueued)
  {
    return;
  }
  fitQueued = true;
  QTimer::singleShot(0,this,[this]()
  {
    fitQueued = false;
    if (isVisible())
    {
      widen_columns_for_visible_rows();
    }
  });
}
void PostprocessorTableView::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  queue_column_fit();
}
void PostprocessorTableView::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
  queue_column_fit();
}
