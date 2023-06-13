
#ifndef STEPSBCSTEMPERATURESTREE_HPP
#define STEPSBCSTEMPERATURESTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;

class StepsBCsTemperaturesTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  StepsBCsTemperaturesTree(QTreeWidgetItem *parent);
  ~StepsBCsTemperaturesTree();

  void initialize(int step_id_init);

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private:
  bool isInitialized;
  int step_id = -1;

  void addBC(QString bc_id, QString bc_name); // adds a new bc to the tree
  void removeBC(QTreeWidgetItem *bc); // removes the bc from to the tree

  int get_child_id(std::string bc_id); // check if the item for the given bc_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
};

#endif // STEPSBCSTEMPERATURESTREE_HPP