#include "PostprocessorTableModel.hpp"
#include <algorithm>
#include <limits>
#include <utility>
PostprocessorTableModel::PostprocessorTableModel(QObject* parent) : QAbstractTableModel(parent)
{
}
void PostprocessorTableModel::set_data(Postprocessor::ResultTableData data)
{
  beginResetModel();
  values = std::move(data);
  endResetModel();
}
int PostprocessorTableModel::rowCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(std::min(values.rows,static_cast<std::size_t>(std::numeric_limits<int>::max())));
}
int PostprocessorTableModel::columnCount(const QModelIndex& parent) const
{
  return parent.isValid() ? 0 : static_cast<int>(values.columns.size());
}
QVariant PostprocessorTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid() || index.row() < 0 || index.column() < 0 || index.row() >= rowCount() || index.column() >= columnCount() ||
      (role != Qt::DisplayRole && role != Qt::UserRole))
  {
    return {};
  }
  const auto value = QString::fromUtf8(Postprocessor::result_table_cell(values,static_cast<std::size_t>(index.row()),
    static_cast<std::size_t>(index.column())).c_str());
  if (role == Qt::UserRole && values.numeric[static_cast<std::size_t>(index.column())])
  {
    bool valid = false;
    const auto integer = value.toLongLong(&valid);
    if (valid)
    {
      return QVariant::fromValue(integer);
    }
    const auto unsignedInteger = value.toULongLong(&valid);
    if (valid)
    {
      return QVariant::fromValue(unsignedInteger);
    }
    const auto number = value.toDouble(&valid);
    if (valid)
    {
      return number;
    }
  }
  return value;
}
QVariant PostprocessorTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole || section < 0)
  {
    return {};
  }
  if (orientation == Qt::Vertical)
  {
    return section+1;
  }
  return section < columnCount() ? QVariant(QString::fromUtf8(values.columns[static_cast<std::size_t>(section)].c_str())) : QVariant();
}
