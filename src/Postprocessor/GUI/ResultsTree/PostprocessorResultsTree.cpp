#include "PostprocessorResultsTree.hpp"

#include <QSignalBlocker>

PostprocessorResultsTree::PostprocessorResultsTree(std::function<void(const QString&)> select,
  std::function<void(const QString&, bool)> visibility, QWidget* parent)
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
  connect(this, &QTreeWidget::itemChanged, this, [visibility](QTreeWidgetItem* item, int column)
  {
    if (item != nullptr && column == 0 && item->flags().testFlag(Qt::ItemIsUserCheckable))
    {
      visibility(item->text(0), item->checkState(0) == Qt::Checked);
    }
  });
}

void PostprocessorResultsTree::refresh(const Postprocessor::PostprocessorSession& session,
  const std::function<bool(const std::string&)>& has_mesh)
{
  const QSignalBlocker blocker(this);
  clear();

  for (const auto& entry : session.sources)
  {
    const auto& source = entry.second;
    auto* item = new QTreeWidgetItem(this, {QString::fromUtf8(source.name.c_str())});
    item->setData(0, Qt::UserRole, QString::fromStdString(source.id));
    if (has_mesh(source.id))
    {
      bool visible = false;
      for (const auto& display : session.representations)
      {
        if (display.second.input.owner == Postprocessor::OutputOwner::Source &&
            display.second.input.object_id == source.id && display.second.input.port == 0 &&
            display.second.view_id == session.active_view_id && display.second.visible)
        {
          visible = true;
          break;
        }
      }
      item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
      item->setCheckState(0, visible ? Qt::Checked : Qt::Unchecked);
    }
    else
    {
      item->setFlags(item->flags() & ~Qt::ItemIsUserCheckable);
      item->setToolTip(0, tr("This source has no displayable mesh increment."));
    }

    if (source.id == session.active_source_id)
    {
      setCurrentItem(item);
    }
  }
}
