
#ifndef CUSTOMLINESTREE_HPP
#define CUSTOMLINESTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class CustomLinesTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  CustomLinesTree(QTreeWidget *parent);
  ~CustomLinesTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addCustomLine(QString customline_id, QString customline_name); // adds a new customline to the tree
  void removeCustomLine(QTreeWidgetItem *customline); // removes the customline from to the tree

  int get_child_id(std::string customline_id); // check if the item for the given customline_id exists, returns the id or -1 if failed;

};

#endif // CUSTOMLINESTREE_HPP