#ifndef DAMPINGTREE_HPP
#define DAMPINGTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class DampingTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  DampingTree(QTreeWidget *parent);
  ~DampingTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addDamping(QString damping_id, QString name); // adds a new damping to the tree
  void removeDamping(QTreeWidgetItem *damping); // removes the damping from to the tree

  int get_child_id(std::string damping_id); // check if the item for the given damping_id exists, returns the id or -1 if failed;

};

#endif // DAMPINGTREE_HPP