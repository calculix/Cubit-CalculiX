#ifndef DAMPINGTREE_HPP
#define DAMPINGTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class DampingTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
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

  CalculiXCoreInterface *ccx_iface;
};

#endif // DAMPINGTREE_HPP