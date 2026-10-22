
#ifndef AMPLITUDESTREE_HPP
#define AMPLITUDESTREE_HPP
#include <string>

#include <QObject>
#include "GUI/common/TreeContextMenu.hpp"
#include <QTreeWidgetItem>
#include <QIcon>
class AmplitudesTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
  Q_OBJECT

public:
  void populateContextMenu(
    QMenu &menu, QTreeWidgetItem *clickedItem, ModelTree &host) override;

  AmplitudesTree(QTreeWidget *parent);
  ~AmplitudesTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;

  void addAmplitude(QString amplitude_id, QString amplitude_name); // adds a new amplitude to the tree
  void removeAmplitude(QTreeWidgetItem *amplitude); // removes the amplitude from to the tree

  int get_child_id(std::string amplitude_id); // check if the item for the given amplitude_id exists, returns the id or -1 if failed;

};

#endif // AMPLITUDESTREE_HPP
