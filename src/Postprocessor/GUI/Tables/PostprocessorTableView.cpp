#include "PostprocessorTableView.hpp"
#include "PostprocessorTableModel.hpp"
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVBoxLayout>
#include <utility>
PostprocessorTableView::PostprocessorTableView(QWidget* parent) : QWidget(parent)
{
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  title = new QLabel(this);
  title->setTextFormat(Qt::PlainText);
  title->setTextInteractionFlags(Qt::TextSelectableByMouse);
  layout->addWidget(title);
  model = new PostprocessorTableModel(this);
  auto* sorted = new QSortFilterProxyModel(this);
  sorted->setSourceModel(model);
  sorted->setSortRole(Qt::UserRole);
  auto* table = new QTableView(this);
  table->setObjectName("Postprocessor.TableView");
  table->setModel(sorted);
  // Header sorting awaits its command contract; retain source row order.
  table->setSortingEnabled(false);
  table->setAlternatingRowColors(true);
  table->setSelectionBehavior(QAbstractItemView::SelectRows);
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  layout->addWidget(table,1);
}
void PostprocessorTableView::show_data(Postprocessor::ResultTableData data)
{
  title->setText(QString::fromUtf8((data.title+(data.status.empty() ? "" : " — "+data.status)).c_str()));
  model->set_data(std::move(data));
}
