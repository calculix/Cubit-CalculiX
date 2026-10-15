#ifndef TOPOLOGYACTIONSPANEL_HPP
#define TOPOLOGYACTIONSPANEL_HPP

#include "CalculiXCoreInterface.hpp"
#include <QWidget>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QShowEvent;

class TopologyActionsPanel : public QWidget
{
public:
  explicit TopologyActionsPanel(QWidget *parent = nullptr);
  void setTopology(int topology_id);

protected:
  void showEvent(QShowEvent *event) override;

private:
  CalculiXCoreInterface ccx_iface;
  QComboBox *cases;
  QCheckBox *wait;
  QLabel *status;
  QPushButton *runButton;
  QPushButton *waitButton;
  QPushButton *killButton;
  QPushButton *monitorButton;

  void refreshCases(int preferred_id = -1);
  void updateControls();
  void execute(const QString& action);
  void openJob();
};

#endif // TOPOLOGYACTIONSPANEL_HPP
