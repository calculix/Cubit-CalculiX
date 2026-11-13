#ifndef POSTPROCESSOR_TABLEMODEL_HPP
#define POSTPROCESSOR_TABLEMODEL_HPP
#include "Postprocessor/Core/tables/ResultTables.hpp"
#include <QAbstractTableModel>
class PostprocessorTableModel : public QAbstractTableModel
{
public:
  explicit PostprocessorTableModel(QObject* parent);
  static constexpr int ActiveFrameRole = Qt::UserRole+1;
  void set_data(Postprocessor::ResultTableData data);
  void set_active_frame(std::optional<std::size_t> frame);
  int rowCount(const QModelIndex& parent = {}) const override;
  int columnCount(const QModelIndex& parent = {}) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
  Postprocessor::ResultTableData values;
};
#endif
