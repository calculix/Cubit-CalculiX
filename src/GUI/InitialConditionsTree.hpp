
#ifndef INITIALCONDITIONSTREE_HPP
#define INITIALCONDITIONSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class InitialConditionsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  InitialConditionsTree(QTreeWidget *parent);
  ~InitialConditionsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addInitialCondition(QString initialcondition_id, QString initialcondition_name); // adds a new initialcondition to the tree
  void removeInitialCondition(QTreeWidgetItem *output); // removes the initialcondition from to the tree

  int get_child_id(std::string initialcondition_id); // check if the item for the given initialcondition_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // INITIALCONDITIONSTREE_HPP