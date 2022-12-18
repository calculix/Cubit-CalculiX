
#ifndef MATERIALTREE_HPP
#define MATERIALTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class MaterialTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  MaterialTree(QTreeWidget *parent);
  ~MaterialTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addMaterial(QString material_id, QString material_name); // adds a new material to the tree
  void removeMaterial(QTreeWidgetItem *material); // removes the material from to the tree

  int get_child_id(std::string material_id); // check if the item for the given material_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // MATERIALTREE_HPP

