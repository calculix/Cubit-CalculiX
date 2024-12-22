#ifndef PHYSICALCONSTANTSTREE_HPP
#define PHYSICALCONSTANTSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class PhysicalConstantsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  PhysicalConstantsTree(QTreeWidget *parent);
  ~PhysicalConstantsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addPhysicalConstants(QString physicalconstants_id, QString name); // adds a new physicalconstants to the tree
  void removePhysicalConstants(QTreeWidgetItem *physicalconstants); // removes the physicalconstants from to the tree

  int get_child_id(std::string physicalconstants_id); // check if the item for the given physicalconstants_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // PHYSICALCONSTANTSTREE_HPP