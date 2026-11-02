#ifndef TOPOLOGYDELETEPANEL_HPP
#define TOPOLOGYDELETEPANEL_HPP
#include <QWidget>

class QComboBox;
class QPushButton;
class QShowEvent;

class TopologyDeletePanel : public QWidget
{
public:
  void setTopology(int topology_id);
  explicit TopologyDeletePanel(QWidget *parent = nullptr);

protected:
  void showEvent(QShowEvent *event) override;

private:
  QComboBox *cases;
  QPushButton *deleteButton;

  void refreshCases(int preferred_id = -1);
  void updateButton();
  void deleteCase();
};

#endif // TOPOLOGYDELETEPANEL_HPP
