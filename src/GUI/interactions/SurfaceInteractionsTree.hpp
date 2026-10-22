
#ifndef SURFACEINTERACTIONSTREE_HPP
#define SURFACEINTERACTIONSTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class SurfaceInteractionsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  SurfaceInteractionsTree(QTreeWidget *parent);
  ~SurfaceInteractionsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addSurfaceInteraction(QString surfaceinteraction_id, QString surfaceinteraction_name); // adds a new surfaceinteraction to the tree
  void removeSurfaceInteraction(QTreeWidgetItem *surfaceinteraction); // removes the surfaceinteraction from to the tree

  int get_child_id(std::string surfaceinteraction_id); // check if the item for the given surfaceinteraction_id exists, returns the id or -1 if failed;

};

#endif // SURFACEINTERACTIONSTREE_HPP
