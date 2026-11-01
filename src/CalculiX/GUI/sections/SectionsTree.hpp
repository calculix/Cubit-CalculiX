
#ifndef SECTIONSTREE_HPP
#define SECTIONSTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class SectionsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  SectionsTree(QTreeWidget *parent);
  ~SectionsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addSection(QString section_id, QString section_name); // adds a new section to the tree
  void removeSection(QTreeWidgetItem *section); // removes the section from to the tree

  int get_child_id(std::string section_id); // check if the item for the given section_id exists, returns the id or -1 if failed;

};

#endif // SECTIONSTREE_HPP
