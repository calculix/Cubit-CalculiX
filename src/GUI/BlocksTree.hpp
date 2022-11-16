
#ifndef BLOCKSTREE_HPP
#define BLOCKSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class BlocksTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  //BlocksTree(QTreeWidget *parent, int type=0);
  BlocksTree(QTreeWidget *parent);
  ~BlocksTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addBlock(QString block_id, QString block_name, QString element_type); // adds a new block to the tree
  void removeBlock(QTreeWidgetItem *block); // removes the block from to the tree

  int get_child_id(std::string block_id); // check if the item for the given block_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // BLOCKSTREE_HPP
