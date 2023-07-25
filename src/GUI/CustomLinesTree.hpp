
#ifndef CUSTOMLINESTREE_HPP
#define CUSTOMLINESTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class CustomLinesTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
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

  CalculiXCoreInterface *ccx_iface;
};

#endif // CUSTOMLINESTREE_HPP