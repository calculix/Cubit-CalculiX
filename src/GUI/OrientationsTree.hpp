#ifndef ORIENTATIONSTREE_HPP
#define ORIENTATIONSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class OrientationsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
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

  CalculiXCoreInterface *ccx_iface;
};

#endif // ORIENTATIONSTREE_HPP