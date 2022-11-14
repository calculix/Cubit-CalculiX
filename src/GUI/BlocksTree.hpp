
#ifndef BLOCKSTREE_HPP
#define BLOCKSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class BlocksTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  //BlocksTree(QTreeWidget *parent, int type=0);
  BlocksTree(QTreeWidget *parent);
  ~BlocksTree();

  void initialize();

  // Cleanup (for example, when we unload the component)
  void clear();

private:
  bool isInitialized;
  
};

#endif // BLOCKSTREE_HPP
