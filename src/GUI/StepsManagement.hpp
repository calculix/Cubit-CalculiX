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

class StepsManagement : public QWidget
{
  Q_OBJECT
  
public:
  StepsManagement();
  ~StepsManagement();

  CalculiXCoreInterface *ccx_iface;
  
  void clear(); // remove all data
  void update();
  void addStep(QString step_id, QString step_name); // adds a new step to the tree
  void removeStep(QTreeWidgetItem *step); // removes the step from to the tree
  int get_child_id(std::string step_id); // check if the item for the given step_id exists, returns the id or -1 if failed;
  void createItems(QTreeWidgetItem *step); // creates the list items for selected step
  void removeItems(); // removes current tree items
  void switchItem(QTreeWidgetItem* currentItem, QTreeWidgetItem* source, QTreeWidgetItem* target); // switches the current item from src tree to target tree
  void setStep(QString step_id);

private slots:
  void on_pushButton_ok_clicked(bool);
  void on_pushButton_apply_clicked(bool);
  void on_pushButton_close_clicked(bool);
  void on_pushButton_add_clicked(bool);
  void on_pushButton_remove_clicked(bool);
  void step_clicked(QTreeWidgetItem* item, int column);
  void step_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item);
  void available_doubleclicked(QTreeWidgetItem* item, int column);
  void used_doubleclicked(QTreeWidgetItem* item, int column);
  
private:
  std::string log;
  QTreeWidgetItem* current_step_item;
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
  QTreeWidgetItem* tree_available_loads_heatfluxes;
  QTreeWidgetItem* tree_available_loads_gravity;
  QTreeWidgetItem* tree_available_loads_centrifugal;
  QTreeWidgetItem* tree_available_loads_trajectory;
  QTreeWidgetItem* tree_available_loads_film;
  QTreeWidgetItem* tree_available_loads_radiation;
  QTreeWidgetItem* tree_available_bcs;
  QTreeWidgetItem* tree_available_bcs_displacements;
  QTreeWidgetItem* tree_available_bcs_temperatures;
  QTreeWidgetItem* tree_available_historyoutputs;
  QTreeWidgetItem* tree_available_fieldoutputs;
  QTreeWidget* tree_used;
  QTreeWidgetItem* tree_used_loads;
  QTreeWidgetItem* tree_used_loads_forces;
  QTreeWidgetItem* tree_used_loads_pressures;
  QTreeWidgetItem* tree_used_loads_heatfluxes;
  QTreeWidgetItem* tree_used_loads_gravity;
  QTreeWidgetItem* tree_used_loads_centrifugal;
  QTreeWidgetItem* tree_used_loads_trajectory;
  QTreeWidgetItem* tree_used_loads_film;
  QTreeWidgetItem* tree_used_loads_radiation;
  QTreeWidgetItem* tree_used_bcs;
  QTreeWidgetItem* tree_used_bcs_displacements;
  QTreeWidgetItem* tree_used_bcs_temperatures;
  QTreeWidgetItem* tree_used_historyoutputs;
  QTreeWidgetItem* tree_used_fieldoutputs;
  
  std::vector<QTreeWidgetItem*> available_trees;
  std::vector<QTreeWidgetItem*> used_trees;
};

#endif // STEPSMANAGEMENT_HPP