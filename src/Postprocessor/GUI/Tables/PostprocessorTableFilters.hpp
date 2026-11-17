#pragma once
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include <QSortFilterProxyModel>
#include <QString>
class QWidget;
class PostprocessorTableFilterProxy : public QSortFilterProxyModel
{
public:
  explicit PostprocessorTableFilterProxy(QObject* parent);
  void set_table(const Postprocessor::ResultTableData& data);
  QVariant headerData(int section,Qt::Orientation orientation,int role = Qt::DisplayRole) const override;
protected:
  bool filterAcceptsRow(int row,const QModelIndex& parent) const override;
private:
  Postprocessor::ResultTableData values;
  std::vector<Postprocessor::PreparedTableFilter> prepared;
};
// Edits a local draft only. The caller journals the accepted filter.
bool edit_table_column_filter(QWidget* parent,const Postprocessor::ResultTableData& data,std::size_t column,
  Postprocessor::TableFilter& filter);
QString table_filter_clause(const Postprocessor::TableFilter& filter);
