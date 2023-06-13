
#ifndef STEPSBCSDISPLACEMENTSTREE_HPP
#define STEPSBCSDISPLACEMENTSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class StepsBCsDisplacementsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  StepsBCsDisplacementsTree(QTreeWidgetItem *parent);
  ~StepsBCsDisplacementsTree();

  void initialize(int step_id_init);

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;
  int step_id = -1;

  void addBC(QString bc_id, QString bc_name); // adds a new bc to the tree
  void removeBC(QTreeWidgetItem *bc); // removes the bc from to the tree

  int get_child_id(std::string bc_id); // check if the item for the given bc_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // STEPSBCSDISPLACEMENTSTREE_HPP