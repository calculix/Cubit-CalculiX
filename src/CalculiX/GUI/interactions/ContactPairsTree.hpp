
#ifndef CONTACTPAIRSTREE_HPP
#define CONTACTPAIRSTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class ContactPairsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  ContactPairsTree(QTreeWidget *parent);
  ~ContactPairsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addContactPair(QString contactpair_id, QString contactpair_name); // adds a new contactpair to the tree
  void removeContactPair(QTreeWidgetItem *contactpair); // removes the contactpair from to the tree

  int get_child_id(std::string contactpair_id); // check if the item for the given contactpair_id exists, returns the id or -1 if failed;

};

#endif // CONTACTPAIRSTREE_HPP
