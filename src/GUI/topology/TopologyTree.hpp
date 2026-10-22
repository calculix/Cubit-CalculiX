#ifndef TOPOLOGYTREE_HPP
#define TOPOLOGYTREE_HPP

#include "GUI/common/TreeContextMenu.hpp"
#include <QObject>
#include <QTreeWidgetItem>
class QTimer;

class TopologyTree : public QObject, public QTreeWidgetItem, public TreeContextMenu
{
public:
  explicit TopologyTree(QTreeWidget *parent);
  ~TopologyTree() override;

  void initialize();
  void update();
  void clear();
  void activate(QTreeWidgetItem *item, ModelTree &host);
  void populateContextMenu(QMenu &menu, QTreeWidgetItem *item, ModelTree &host) override;

private:
  enum Kind { Root, Case, Iterations, Iteration, Job };
  enum Role { KindRole = Qt::UserRole, IdRole, TopologyRole, IterationRole, JobRole, CompletedRole };

  QTimer *refreshTimer = nullptr;
  QTreeWidgetItem *ensure(QTreeWidgetItem *parent, Kind kind, int id, const QString &label);
  void preview(QTreeWidgetItem *item);
  static void open_job(int job_id, ModelTree &host);
  static void open_settings(int topology_id, ModelTree &host);
  static void open_preserved_sets(int topology_id, ModelTree &host);
  static void open_results(int topology_id, int iteration_id, ModelTree &host);
  static void open_actions(int topology_id, ModelTree &host);
};

#endif // TOPOLOGYTREE_HPP
