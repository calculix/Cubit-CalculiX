
#ifndef CONSTRAINTSTREE_HPP
#define CONSTRAINTSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class ConstraintsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  ConstraintsTree(QTreeWidget *parent);
  ~ConstraintsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addConstraint(QString constraint_id, QString constraint_name); // adds a new constraint to the tree
  void removeConstraint(QTreeWidgetItem *constraint); // removes the constraint from to the tree

  int get_child_id(std::string constraint_id); // check if the item for the given constraint_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // CONSTRAINTSTREE_HPP
