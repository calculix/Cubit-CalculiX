#ifndef POSTPROCESSOR_TABLEMODEL_HPP
#define POSTPROCESSOR_TABLEMODEL_HPP
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include <QAbstractTableModel>
class PostprocessorTableModel : public QAbstractTableModel
{
public:
  explicit PostprocessorTableModel(QObject* parent);
  void set_data(Postprocessor::ResultTableData data);
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
  Postprocessor::ResultTableData values;
};
#endif
