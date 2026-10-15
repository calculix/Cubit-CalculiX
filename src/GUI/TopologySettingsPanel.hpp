#ifndef TOPOLOGYSETTINGSPANEL_HPP
#define TOPOLOGYSETTINGSPANEL_HPP

#include "CalculiXCoreInterface.hpp"
#include <QWidget>
#include <QStringList>
#include <array>

class QComboBox;
class QLineEdit;
class QPushButton;
class QShowEvent;
class QSpinBox;

class TopologySettingsPanel : public QWidget
{
public:
  explicit TopologySettingsPanel(bool modify, QWidget *parent = nullptr);
  bool isModify() const;
  void setTopology(int topology_id);

protected:
  void showEvent(QShowEvent *event) override;

private:
  bool modifying;
  CalculiXCoreInterface ccx_iface;
  QSpinBox *id;
  QComboBox *cases;
  QWidget *settings;
  // Name, volume fraction, void ratio, maximum iterations, filter radius.
  std::array<QLineEdit *, 5> fields;
  QComboBox *method;
  QComboBox *objective;
  QPushButton *applyButton;
  QStringList original;
  int loaded_id = -1;

  void refreshCases(int preferred_id = -1);
  void load();
  void apply();
  QStringList values() const;
};

#endif // TOPOLOGYSETTINGSPANEL_HPP
