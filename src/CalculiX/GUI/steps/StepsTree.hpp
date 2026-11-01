#ifndef STEPSTREE_HPP
#define STEPSTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class StepsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  StepsTree(QTreeWidget *parent);
  ~StepsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addStep(QString step_id, QString step_name); // adds a new step to the tree
  void removeStep(QTreeWidgetItem *step); // removes the step from to the tree

  int get_child_id(std::string step_id); // check if the item for the given step_id exists, returns the id or -1 if failed;

};

#endif // STEPSTREE_HPP