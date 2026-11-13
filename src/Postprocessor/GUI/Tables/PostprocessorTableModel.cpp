#include "PostprocessorTableModel.hpp"
#include <algorithm>
#include <limits>
#include <utility>
#include <QBrush>
#include <QColor>
#include <QFont>
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
      (role != Qt::DisplayRole && role != Qt::UserRole && role != Qt::ToolTipRole && role != Qt::BackgroundRole &&
       role != Qt::ForegroundRole && role != Qt::FontRole && role != ActiveFrameRole))
  {
    return {};
  }
  if (role == Qt::BackgroundRole || role == Qt::ForegroundRole || role == Qt::FontRole || role == ActiveFrameRole)
  {
    const auto frame = Postprocessor::result_table_row_frame(values,static_cast<std::size_t>(index.row()));
    const bool active = frame && values.active_frame && frame == values.active_frame;
    if (role == ActiveFrameRole)
    {
      return active;
    }
    if (!active)
    {
      return {};
    }
    if (role == Qt::FontRole)
    {
      QFont font;
      font.setBold(true);
      return font;
    }
    const auto& color = Postprocessor::TableDefaults::active_frame_color;
    return QBrush(role == Qt::ForegroundRole ? QColor(0,0,0) : QColor(color[0],color[1],color[2]));
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
  if ((role != Qt::DisplayRole && role != Qt::ToolTipRole) || section < 0)
  {
    return {};
  }
  if (orientation == Qt::Vertical)
  {
    return section+1;
  }
  return section < columnCount() ? QVariant(QString::fromUtf8(values.columns[static_cast<std::size_t>(section)].c_str())) : QVariant();
}

void PostprocessorTableModel::set_active_frame(std::optional<std::size_t> frame)
{
  if (values.active_frame == frame)
  {
    return;
  }
  const auto previous = values.active_frame;
  values.active_frame = frame;
  for (const auto& part : values.parts)
  {
    if (!part.frame || (part.frame != previous && part.frame != frame) || !part.count ||
        part.first_row >= static_cast<std::size_t>(rowCount()) || !columnCount())
    {
      continue;
    }
    const int first = static_cast<int>(part.first_row);
    const int last = static_cast<int>(std::min(part.first_row+part.count-1,static_cast<std::size_t>(rowCount()-1)));
    emit dataChanged(index(first,0),index(last,columnCount()-1),
      {Qt::BackgroundRole,Qt::ForegroundRole,Qt::FontRole,ActiveFrameRole});
  }
}
