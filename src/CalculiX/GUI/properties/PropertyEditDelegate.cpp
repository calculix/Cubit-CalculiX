#include "PropertyEditDelegate.hpp"
#include <QApplication>
#include <QAbstractItemView>
#include <QMouseEvent>
#include <QComboBox>
#include <QLineEdit>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTimer>
#include <utility>

PropertyEditDelegate::PropertyEditDelegate(QObject* parent, std::function<void(const PropertyEditRequest&)> commit)
  : QStyledItemDelegate(parent), commit(std::move(commit))
{
}

QWidget* PropertyEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
  if (index.column() != 1)
  {
    return nullptr;
  }
  const auto kind = static_cast<PropertyEditorKind>(index.data(PropertyEditorRole).toInt());
  if (kind == PropertyEditorKind::Name)
  {
    return new QLineEdit(parent);
  }
  if (kind == PropertyEditorKind::Choice)
  {
    const auto choices = index.data(PropertyChoicesRole).toStringList();
    if (choices.isEmpty())
    {
      return nullptr;
    }
    auto* combo = new QComboBox(parent);
    combo->addItems(choices);
    auto* delegate = const_cast<PropertyEditDelegate*>(this);
    connect(combo, &QComboBox::activated, delegate, [delegate, combo](int)
    {
      emit delegate->commitData(combo);
      emit delegate->closeEditor(combo);
    });
    QTimer::singleShot(0, combo, &QComboBox::showPopup);
    return combo;
  }
  return nullptr;
}

void PropertyEditDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
  const QString original = index.data(PropertyOriginalRole).toString();
  if (auto* text = qobject_cast<QLineEdit*>(editor))
  {
    text->setText(original);
    text->selectAll();
  }
  else if (auto* combo = qobject_cast<QComboBox*>(editor))
  {
    combo->setPlaceholderText(original.isEmpty() ? tr("Select an element type") : original);
    combo->setCurrentIndex(combo->findText(original, Qt::MatchFixedString));
  }
}

void PropertyEditDelegate::setModelData(QWidget* editor, QAbstractItemModel*, const QModelIndex& index) const
{
  PropertyEditRequest request;
  request.entityId = index.data(PropertyEntityRole).toInt();
  request.revision = index.data(PropertyRevisionRole).toULongLong();
  request.field = index.data(PropertyFieldRole).toString();
  request.original = index.data(PropertyOriginalRole).toString();
  if (auto* text = qobject_cast<QLineEdit*>(editor))
  {
    request.value = text->text();
  }
  else if (auto* combo = qobject_cast<QComboBox*>(editor))
  {
    if (combo->currentIndex() < 0)
    {
      return;
    }
    request.value = combo->currentText();
  }
  else
  {
    return;
  }
  if (request.value == request.original || !commit)
  {
    return;
  }
  // Command completion can rebuild the view. Run only after Qt closes the editor.
  // Keep the displayed value unchanged until the feature reloads authoritative core data.
  const auto handler = commit;
  QTimer::singleShot(0, parent(), [handler, request]() { handler(request); });
}

bool PropertyEditDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
{
  if (index.column() == 1 && static_cast<PropertyEditorKind>(index.data(PropertyEditorRole).toInt()) == PropertyEditorKind::Choice && event->type() == QEvent::MouseButtonRelease)
  {
    const auto* mouse = static_cast<QMouseEvent*>(event);
    const QRect arrow(option.rect.right() - 20, option.rect.top(), 20, option.rect.height());
    if (mouse->button() == Qt::LeftButton && arrow.contains(mouse->position().toPoint()))
    {
      if (auto* view = qobject_cast<QAbstractItemView*>(parent()))
      {
        view->edit(index);
        return true;
      }
    }
  }
  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void PropertyEditDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter, option, index);
  if (index.column() == 1 && static_cast<PropertyEditorKind>(index.data(PropertyEditorRole).toInt()) == PropertyEditorKind::Choice)
  {
    QStyleOption arrow;
    arrow.rect = QRect(option.rect.right() - 16, option.rect.center().y() - 5, 10, 10);
    arrow.state = option.state;
    arrow.palette = option.palette;
    auto* style = option.widget == nullptr ? QApplication::style() : option.widget->style();
    style->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrow, painter, option.widget);
  }
}
