#include "PostprocessorResultsTree.hpp"

#include <QSignalBlocker>

PostprocessorResultsTree::PostprocessorResultsTree(std::function<void(const QString&)> select, QWidget* parent)
  : QTreeWidget(parent)
{
  setObjectName("Postprocessor.ResultsTree");
  setHeaderLabel(tr("Sources / Results"));
  setSelectionMode(QAbstractItemView::SingleSelection);
  connect(this, &QTreeWidget::currentItemChanged, this, [select](QTreeWidgetItem* current, QTreeWidgetItem*)
  {
    if (current != nullptr)
    {
      select(current->text(0));
    }
  });
}

void PostprocessorResultsTree::refresh(const Postprocessor::PostprocessorSession& session)
{
  const QSignalBlocker blocker(this);
  clear();

  for (const auto& entry : session.sources)
  {
    const auto& source = entry.second;
    auto* item = new QTreeWidgetItem(this, {QString::fromUtf8(source.name.c_str())});
    item->setData(0, Qt::UserRole, QString::fromStdString(source.id));

    if (source.id == session.active_source_id)
    {
      setCurrentItem(item);
    }
  }
}
