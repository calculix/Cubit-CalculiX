#ifndef TOPOLOGYACTIONSPANEL_HPP
#define TOPOLOGYACTIONSPANEL_HPP

#include "CalculiXCoreInterface.hpp"
#include <QWidget>
#include <array>

class QCheckBox;
class QComboBox;
class QLabel;
class QPushButton;
class QShowEvent;
class QProgressBar;
class QVBoxLayout;

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

  struct ConvergenceCriterion
  {
    QProgressBar *bar = nullptr;
    QLabel *badge = nullptr;
  };

  std::array<ConvergenceCriterion, 3> criteria;
  QLabel *convergenceIteration;
  QLabel *convergenceSummary;
  QLabel *displacementMeasurement;

  void createConvergenceDisplay(QVBoxLayout *layout);
  void updateConvergence();
  int updateCriterion(int index, const QString& value, double limit);

  void refreshCases(int preferred_id = -1);
  void updateControls();
  void execute(const QString& action);
  void openJob();
};

#endif // TOPOLOGYACTIONSPANEL_HPP
