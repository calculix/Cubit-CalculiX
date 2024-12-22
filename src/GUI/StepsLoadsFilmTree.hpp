
#ifndef STEPSLOADSFILMTREE_HPP
#define STEPSLOADSFILMTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class StepsLoadsFilmTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  StepsLoadsFilmTree(QTreeWidgetItem *parent);
  ~StepsLoadsFilmTree();

  void initialize(int step_id_init);

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;
  int step_id = -1;

  void addLoad(QString load_id, QString load_name); // adds a new load to the tree
  void removeLoad(QTreeWidgetItem *load); // removes the load from to the tree

  int get_child_id(std::string load_id); // check if the item for the given load_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // STEPSLOADSFILMTREE_HPP