#include "PostprocessorStyleDelegate.hpp"
#include <QComboBox>
#include <QPainter>
#include <QPointer>
#include <QStyle>
#include <QStyleOption>
#include <QApplication>
#include <utility>
QString appearance_style_label(Postprocessor::DisplayStyle style)
{
  switch (style)
  {
    case Postprocessor::DisplayStyle::Wireframe: return QObject::tr("Wireframe");
    case Postprocessor::DisplayStyle::HiddenLine: return QObject::tr("Hidden line");
    case Postprocessor::DisplayStyle::Shaded: return QObject::tr("Shaded");
    case Postprocessor::DisplayStyle::Transparent: return QObject::tr("Transparent");
    case Postprocessor::DisplayStyle::Points: return QObject::tr("Points");
    case Postprocessor::DisplayStyle::RoundedPoints: return QObject::tr("Rounded points");
  }
  return {};
}
QIcon appearance_style_icon(Postprocessor::DisplayStyle style)
{
  switch (style)
  {
    case Postprocessor::DisplayStyle::Wireframe: return QIcon(":/cubitgui/wire.svg");
    case Postprocessor::DisplayStyle::HiddenLine: return QIcon(":/cubitgui/hidden.svg");
    case Postprocessor::DisplayStyle::Shaded: return QIcon(":/cubitgui/shaded.svg");
    case Postprocessor::DisplayStyle::Transparent: return QIcon(":/cubitgui/trans.svg");
    case Postprocessor::DisplayStyle::Points: return QIcon(":/cubitgui/node.svg");
    case Postprocessor::DisplayStyle::RoundedPoints: return QIcon(":/cubitgui/node.svg");
  }
  return {};
}
PostprocessorStyleDelegate::PostprocessorStyleDelegate(std::function<void(const QModelIndex&,Postprocessor::DisplayStyle)> callback,QObject* parent)
  : QStyledItemDelegate(parent),change(std::move(callback))
{
}
QWidget* PostprocessorStyleDelegate::createEditor(QWidget* parent,const QStyleOptionViewItem&,const QModelIndex& index) const
{
  if (!index.data(PostprocessorTreeRoles::Style).isValid())
  {
    return nullptr;
  }
  auto* editor = new QComboBox(parent);
  SimResults::Dataset dataset;
  dataset.kind = index.data(PostprocessorTreeRoles::DatasetKind).toString().toStdString();
  for (auto style : {Postprocessor::DisplayStyle::Wireframe,Postprocessor::DisplayStyle::HiddenLine,
      Postprocessor::DisplayStyle::Shaded,Postprocessor::DisplayStyle::Transparent,
      Postprocessor::DisplayStyle::Points,Postprocessor::DisplayStyle::RoundedPoints})
  {
    if (Postprocessor::supports_style(dataset,style))
    {
      editor->addItem(appearance_style_icon(style),appearance_style_label(style),static_cast<int>(style));
    }
  }
  auto* self = const_cast<PostprocessorStyleDelegate*>(this);
  QObject::connect(editor,&QComboBox::activated,self,[self,editor](int)
  {
    QPointer<QComboBox> guarded(editor);
    editor->setProperty("Postprocessor.StyleActivated",true);
    emit self->commitData(editor);
    if (guarded)
    {
      emit self->closeEditor(guarded);
    }
  });
  return editor;
}
void PostprocessorStyleDelegate::setEditorData(QWidget* widget,const QModelIndex& index) const
{
  auto* editor = qobject_cast<QComboBox*>(widget);
  if (editor)
  {
    editor->setCurrentIndex(editor->findData(index.data(PostprocessorTreeRoles::Style)));
    editor->setProperty("Postprocessor.StyleActivated",false);
  }
}
void PostprocessorStyleDelegate::setModelData(QWidget* widget,QAbstractItemModel*,const QModelIndex& index) const
{
  auto* editor = qobject_cast<QComboBox*>(widget);
  if (editor && editor->currentIndex() >= 0 && index.isValid() &&
      index.data(PostprocessorTreeRoles::Style).isValid() &&
      editor->property("Postprocessor.StyleActivated").toBool() && change)
  {
    // Focus loss or tree refresh must not commit an untouched editor. Explicit
    // activation may apply this row's style to differently styled selected rows.
    editor->setProperty("Postprocessor.StyleActivated",false);
    change(index,static_cast<Postprocessor::DisplayStyle>(editor->currentData().toInt()));
  }
}
void PostprocessorStyleDelegate::paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const
{
  QStyledItemDelegate::paint(painter,option,index);
  if (index.data(PostprocessorTreeRoles::Style).isValid())
  {
    QStyleOption indicator;
    indicator.rect = QRect(option.rect.right()-10,option.rect.center().y()-3,7,6);
    indicator.state = QStyle::State_Enabled;
    QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowDown,&indicator,painter);
  }
}
