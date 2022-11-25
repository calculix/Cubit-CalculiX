
#ifndef NODESETTREE_HPP
#define NODESETTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class NodesetTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  //NodesetTree(QTreeWidget *parent, int type=0);
  NodesetTree(QTreeWidget *parent);
  ~NodesetTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addNodeset(QString nodeset_id, QString nodeset_name); // adds a new nodeset to the tree
  void removeNodeset(QTreeWidgetItem *nodeset); // removes the nodeset from to the tree

  int get_child_id(std::string nodeset_id); // check if the item for the given block_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // NODESETTREE_HPP
