#ifndef STEPSMANAGEMENT_HPP
#define STEPSMANAGEMENT_HPP

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QString>
#include <QStringList>

class CalculiXCoreInterface;
class StepsManagementItem;

class StepsManagement : public QWidget
{
  Q_OBJECT
  
public:
  StepsManagement();
  ~StepsManagement();

  CalculiXCoreInterface *ccx_iface;
  
  void clear(); // remove all data
  void update();

private slots:
  void on_pushButton_ok_clicked(bool);
  void on_pushButton_apply_clicked(bool);
  void on_pushButton_close_clicked(bool);
  void on_pushButton_add_clicked(bool);
  void on_pushButton_remove_clicked(bool);
  
private:
  std::string log;
  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_steps_1;
  QVBoxLayout* boxLayout_assignment_1;
  QVBoxLayout* boxLayout_assignment_2;
  QVBoxLayout* boxLayout_assignment_3;
  QPushButton* pushButton_ok;
  QPushButton* pushButton_apply;
  QPushButton* pushButton_close;
  QPushButton* pushButton_add;
  QPushButton* pushButton_remove;
  QLabel* label_steps;
  QLabel* label_available;
  QLabel* label_used;
  QTreeWidget* tree_steps;
  QTreeWidget* tree_available;
  QTreeWidgetItem* tree_available_loads;
  QTreeWidgetItem* tree_available_loads_forces;
  QTreeWidgetItem* tree_available_loads_pressures;
  QTreeWidgetItem* tree_available_bcs;
  QTreeWidgetItem* tree_available_bcs_displacements;
  QTreeWidgetItem* tree_available_bcs_temperatures;
  QTreeWidgetItem* tree_available_historyoutputs;
  QTreeWidgetItem* tree_available_fieldoutputs;
  QTreeWidget* tree_used;
  QTreeWidgetItem* tree_used_loads;
  QTreeWidgetItem* tree_used_loads_forces;
  QTreeWidgetItem* tree_used_loads_pressures;
  QTreeWidgetItem* tree_used_bcs;
  QTreeWidgetItem* tree_used_bcs_displacements;
  QTreeWidgetItem* tree_used_bcs_temperatures;
  QTreeWidgetItem* tree_used_historyoutputs;
  QTreeWidgetItem* tree_used_fieldoutputs;
};

#endif // STEPSMANAGEMENT_HPP