#ifndef JOBSMONITORFRD_HPP
#define JOBSMONITORFRD_HPP

#include <QWidget>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QList>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QInputDialog>
#include <QComboBox>
#include <QString>
#include <QStringList>

class CalculiXCoreInterface;

class JobsMonitorFRD : public QWidget
{
  Q_OBJECT
  
public:
  JobsMonitorFRD();
  ~JobsMonitorFRD();

  CalculiXCoreInterface *ccx_iface;

  int current_job_id = -1;
  
  void clear(); // remove all data
  void update();
  void update_component(std::string result_block);
  void set_current_job_id(int job_id);

  void addListItem(std::string item_name, QListWidget* parent_list); // adds a new item to the list
  void removeListItem(std::string item_name, QListWidget* parent_list); // removes the item from the list

  void createListItems(); // creates the list items for selected material
  void removeListItems(); // removes current list items
  void removeListItems_from_List(QListWidget* list); // removes current list items from list
  void selectListItem(QListWidgetItem* item); // unselect all list items except the given

private slots:
  void on_pushButton_refresh_clicked(bool);
  void on_pushButton_plot_clicked(bool);
  void result_block_clicked(QListWidgetItem* item);
  void result_block_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item);
  
private:
  std::string log;
  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_result_block;
  QVBoxLayout* boxLayout_component;
  QVBoxLayout* boxLayout_widget;
  QPushButton* pushButton_refresh;
  QPushButton* pushButton_plot;
  QLabel* label_result_block;
  QLabel* label_component;
  QListWidget* list_result_block;
  QListWidget* list_component;
  // results
  QFrame* result_frame;
};

#endif // JOBSMONITORFRD_HPP