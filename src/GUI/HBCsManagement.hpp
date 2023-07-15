#ifndef HBCSMANAGEMENT_HPP
#define HBCSMANAGEMENT_HPP

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

class HBCsManagement : public QWidget
{
  Q_OBJECT
  
public:
  HBCsManagement();
  ~HBCsManagement();

  CalculiXCoreInterface *ccx_iface;
  
  void clear(); // remove all data
  void update();
  void addHBC(QString hbc_id, QString hbc_name); // adds a new hbc to the tree
  void removeHBC(QTreeWidgetItem *hbc); // removes the hbc from to the tree
  int get_child_id(std::string hbc_id); // check if the item for the given hbc_id exists, returns the id or -1 if failed;
  void createItems(QTreeWidgetItem *hbc); // creates the list items for selected hbc
  void removeItems(); // removes current tree items
  void switchItem(QTreeWidgetItem* currentItem, QTreeWidgetItem* source, QTreeWidgetItem* target); // switches the current item from src tree to target tree
  void setHBC(QString hbc_id);

private slots:
  void on_pushButton_ok_clicked(bool);
  void on_pushButton_apply_clicked(bool);
  void on_pushButton_close_clicked(bool);
  void on_pushButton_add_clicked(bool);
  void on_pushButton_remove_clicked(bool);
  void hbc_clicked(QTreeWidgetItem* item, int column);
  void hbc_changed(QTreeWidgetItem* current_item, QTreeWidgetItem* prev_item);
  void available_doubleclicked(QTreeWidgetItem* item, int column);
  void used_doubleclicked(QTreeWidgetItem* item, int column);
  
private:
  std::string log;
  QTreeWidgetItem* current_hbc_item;
  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_hbcs_1;
  QVBoxLayout* boxLayout_assignment_1;
  QVBoxLayout* boxLayout_assignment_2;
  QVBoxLayout* boxLayout_assignment_3;
  QPushButton* pushButton_ok;
  QPushButton* pushButton_apply;
  QPushButton* pushButton_close;
  QPushButton* pushButton_add;
  QPushButton* pushButton_remove;
  QLabel* label_hbcs;
  QLabel* label_available;
  QLabel* label_used;
  QTreeWidget* tree_hbcs;
  QTreeWidget* tree_available;
  QTreeWidgetItem* tree_available_bcs;
  QTreeWidgetItem* tree_available_bcs_displacements;
  QTreeWidgetItem* tree_available_bcs_temperatures;
  QTreeWidget* tree_used;
  QTreeWidgetItem* tree_used_bcs;
  QTreeWidgetItem* tree_used_bcs_displacements;
  QTreeWidgetItem* tree_used_bcs_temperatures;
  
  std::vector<QTreeWidgetItem*> available_trees;
  std::vector<QTreeWidgetItem*> used_trees;
};

#endif // HBCSMANAGEMENT_HPP