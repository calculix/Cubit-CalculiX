#pragma once
#include <QTreeWidget>
#include "GUI/common/TreeContextMenu.hpp"

class ConnectionsTree : public QTreeWidgetItem, public TreeContextMenu
{
public:
  explicit ConnectionsTree(QTreeWidget* parent);
  void update();
  void clear();
  void populateContextMenu(QMenu& menu, QTreeWidgetItem* item, ModelTree& host) override;
};
