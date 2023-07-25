
#ifndef SECTIONSTREE_HPP
#define SECTIONSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class SectionsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
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

  CalculiXCoreInterface *ccx_iface;
};

#endif // SECTIONSTREE_HPP
