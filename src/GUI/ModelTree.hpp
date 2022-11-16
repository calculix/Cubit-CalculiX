
#ifndef MODELTREE_HPP
#define MODELTREE_HPP

#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPoint>

class BlocksTree;
class QTreeWidgetItem;

class ModelTree : public QTreeWidget
{
  Q_OBJECT

public:
  ModelTree(QDockWidget *parent);
  ~ModelTree();


public slots:
  void showContextMenu(const QPoint &pos);
  void ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column);

};

#endif // MODELTREE_HPP
