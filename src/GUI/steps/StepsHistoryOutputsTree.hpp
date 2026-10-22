
#ifndef STEPSHISTORYOUTPUTSTREE_HPP
#define STEPSHISTORYOUTPUTSTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class StepsHistoryOutputsTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  StepsHistoryOutputsTree(QTreeWidgetItem *parent);
  ~StepsHistoryOutputsTree();

  void initialize(int step_id_init);

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;
  int step_id = -1;

  void addOutput(QString output_id, QString output_name); // adds a new output to the tree
  void removeOutput(QTreeWidgetItem *output); // removes the output from to the tree

  int get_child_id(std::string output_id); // check if the item for the given output_id exists, returns the id or -1 if failed;

};

#endif // STEPSHISTORYOUTPUTSTREE_HPP