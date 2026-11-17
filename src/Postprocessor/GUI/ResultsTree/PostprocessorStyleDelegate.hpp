#pragma once
#include "Postprocessor/Core/representations/DatasetAppearance.hpp"
#include <QStyledItemDelegate>
#include <QIcon>
#include <QString>
#include <functional>
namespace PostprocessorTreeRoles
{
  inline constexpr int Style = Qt::UserRole+20;
  inline constexpr int DatasetKind = Qt::UserRole+21;
}
QString appearance_style_label(Postprocessor::DisplayStyle style);
QIcon appearance_style_icon(Postprocessor::DisplayStyle style);
class PostprocessorStyleDelegate : public QStyledItemDelegate
{
public:
  PostprocessorStyleDelegate(std::function<void(const QModelIndex&,Postprocessor::DisplayStyle)> change,QObject* parent);
  QWidget* createEditor(QWidget* parent,const QStyleOptionViewItem& option,const QModelIndex& index) const override;
  void setEditorData(QWidget* editor,const QModelIndex& index) const override;
  void setModelData(QWidget* editor,QAbstractItemModel* model,const QModelIndex& index) const override;
  void paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const override;
private:
  std::function<void(const QModelIndex&,Postprocessor::DisplayStyle)> change;
};
