
#ifndef JOBSTREE_HPP
#define JOBSTREE_HPP

#include <QObject>
#include <QTreeWidgetItem>
#include <QIcon>

class CalculiXCoreInterface;
class GUITimer;

class JobsTree : public QObject, public QTreeWidgetItem
{
  Q_OBJECT

public:
  JobsTree(QTreeWidget *parent);
  ~JobsTree();

  void initialize();

  void clear(); // remove all children from tree 

  void update(); // updates the children from the tree

private slots:
  void update_icon();

private:
  bool isInitialized;

  void addJob(QString job_id, QString job_name); // adds a new job to the tree
  void removeJob(QTreeWidgetItem *job); // removes the job from to the tree

  int get_child_id(std::string job_id); // check if the item for the given job_id exists, returns the id or -1 if failed;

  CalculiXCoreInterface *ccx_iface;
  GUITimer *timer;
};

#endif // JOBSTREE_HPP