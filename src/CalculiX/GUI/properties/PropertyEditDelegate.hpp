#ifndef PROPERTYEDITDELEGATE_HPP
#define PROPERTYEDITDELEGATE_HPP

#include "PropertyEditRequest.hpp"
#include <QStyledItemDelegate>
#include <functional>

class PropertyEditDelegate : public QStyledItemDelegate
{
public:
  explicit PropertyEditDelegate(QObject* parent, std::function<void(const PropertyEditRequest&)> commit);
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
  bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;
  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
  std::function<void(const PropertyEditRequest&)> commit;
};

#endif
