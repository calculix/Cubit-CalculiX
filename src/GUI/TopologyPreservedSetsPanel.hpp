#ifndef TOPOLOGYPRESERVEDSETSPANEL_HPP
#define TOPOLOGYPRESERVEDSETSPANEL_HPP

#include "CalculiXCoreInterface.hpp"
#include <QWidget>

class QComboBox;
class QLabel;
class QPushButton;
class QShowEvent;
class QTreeWidget;

class TopologyPreservedSetsPanel : public QWidget
{
public:
  explicit TopologyPreservedSetsPanel(QWidget *parent = nullptr);
  void setTopology(int topology_id);

protected:
  void showEvent(QShowEvent *event) override;

private:
  CalculiXCoreInterface ccx_iface;

  QComboBox *cases;
  QComboBox *setType;
  QTreeWidget *available;
  QTreeWidget *preserved;
  QPushButton *addButton;
  QPushButton *removeButton;
  QPushButton *resetButton;
  QLabel *information;

  int loaded_id = -1;
  int loaded_type = 0;
  bool editable = false;

  void refreshCases(int preferred_id = -1);
  void refreshLists();
  void updateButtons();
  void changeSelection(bool adding);
  void resetDefaults();
};

#endif // TOPOLOGYPRESERVEDSETSPANEL_HPP
