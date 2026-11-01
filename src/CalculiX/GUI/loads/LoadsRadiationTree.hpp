
#ifndef LOADSRADIATIONTREE_HPP
#define LOADSRADIATIONTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class LoadsRadiationTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  LoadsRadiationTree(QTreeWidgetItem *parent);
  ~LoadsRadiationTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addLoad(QString load_id, QString load_name); // adds a new load to the tree
  void removeLoad(QTreeWidgetItem *load); // removes the load from to the tree

  int get_child_id(std::string load_id); // check if the item for the given load_id exists, returns the id or -1 if failed;

};

#endif // LOADSRADIATIONTREE_HPP