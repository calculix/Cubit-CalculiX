
#ifndef FIELDOUTPUTSTREE_HPP
#define FIELDOUTPUTSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>

class CalculiXCoreInterface;

class FieldOutputsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  FieldOutputsTree(QTreeWidget *parent);
  ~FieldOutputsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addOutput(QString output_id, QString output_name); // adds a new output to the tree
  void removeOutput(QTreeWidgetItem *output); // removes the output from to the tree

  int get_child_id(std::string output_id); // check if the item for the given output_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // FIELDOUTPUTSTREE_HPP