
#ifndef SIDESETTREE_HPP
#define SIDESETTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class SidesetTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  SidesetTree(QTreeWidget *parent);
  ~SidesetTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addSideset(QString sideset_id, QString sideset_name); // adds a new sideset to the tree
  void removeSideset(QTreeWidgetItem *sideset); // removes the sideset from to the tree

  int get_child_id(std::string sideset_id); // check if the item for the given sideset_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // SIDESETTREE_HPP
