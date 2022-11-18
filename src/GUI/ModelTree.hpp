
#ifndef MODELTREE_HPP
#define MODELTREE_HPP

#include <QWidget>
#include <QDockWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPoint>
#include <QModelIndex>

class BlocksTree;
class QTreeWidgetItem;

class NavigationModel;

class CalculiXCoreInterface;
class Claro;
class CommandButtonPanel;

class ModelTree : public QTreeWidget
{
  Q_OBJECT

public:
  ModelTree(QDockWidget *parent);
  ~ModelTree();

  void setWidgetInCmdPanel(QString name); // set Widget in CommandPanel
  void getWidgetInCmdPanel(); // get current Widget in CommandPanel
  
  NavigationModel* nav_model;
  CalculiXCoreInterface* ccx_iface;
  Claro* gui;
  CommandButtonPanel* cmdpanel;

public slots:
  void showContextMenu(const QPoint &pos);
  void ModelTreeItemDoubleClicked(QTreeWidgetItem* item, int column);

};

#endif // MODELTREE_HPP
