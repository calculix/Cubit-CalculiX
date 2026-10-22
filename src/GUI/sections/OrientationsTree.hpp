#ifndef ORIENTATIONSTREE_HPP
#define ORIENTATIONSTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class OrientationsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  OrientationsTree(QTreeWidget *parent);
  ~OrientationsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addOrientation(QString orientation_id, QString name); // adds a new orientation to the tree
  void removeOrientation(QTreeWidgetItem *orientation); // removes the orientation from to the tree

  int get_child_id(std::string orientation_id); // check if the item for the given orientation_id exists, returns the id or -1 if failed;

};

#endif // ORIENTATIONSTREE_HPP