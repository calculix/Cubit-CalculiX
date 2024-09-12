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
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <algorithm>
#include <QComboBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QObject>

class CalculiXCoreInterface;
class JobsMonitorFRDPlot;
class PickWidget;

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
  void update_increment();
  void update_result();
  void set_current_job_id(int job_id);

  void addListItem(std::string item_name, QListWidget* parent_list); // adds a new item to the list
  void addTableItem(std::string item_name, QTableWidget* parent_list); //adds a new item to the table

private slots:
  void on_pushButton_reset_clicked(bool);
  void on_pushButton_plot_clicked(bool);
  void on_pushButton_query_results_clicked(bool);
  void on_pushButton_previous_clicked(bool);
  void on_pushButton_next_clicked(bool);
  void on_pushButton_export_clicked(bool);
  void update_filter_by_set(int index);
  void result_block_clicked(QListWidgetItem* item);
  void result_block_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item);
  void component_clicked(QListWidgetItem* item);
  void component_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item);
  void increment_clicked(QListWidgetItem* item);
  void increment_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item);

private:
  std::string log;
  QGridLayout* gridLayout;
  QVBoxLayout* boxLayout_result_block;
  QVBoxLayout* boxLayout_component;
  QVBoxLayout* boxLayout_increment;
  QVBoxLayout* boxLayout_filter;
  QVBoxLayout* boxLayout_filter_by_set;
  QHBoxLayout* boxLayout_filter_by_node;
  QHBoxLayout* boxLayout_result_buttons;
  QVBoxLayout* boxLayout_widget;
  QHBoxLayout* boxLayout_pages;
  QPushButton* pushButton_reset;
  QPushButton* pushButton_plot;
  QPushButton* pushButton_query_results;
  QPushButton* pushButton_prev;
  QPushButton* pushButton_next;
  QPushButton* pushButton_export;
  QLabel* label_result_block;
  QLabel* label_component;
  QLabel* label_increment;
  QLabel* label_filter_by_set;
  QLabel* label_filter_by_node;
  QLabel* label_filter_node_id;
  PickWidget* PickWidget_filter_node_id;
  QListWidget* list_result_block;
  QListWidget* list_component;
  QListWidget* list_increment;
  QTableWidget* table_filter_by_set;
  QTableWidget* table_increment;
  QLabel* table_counter;
  QComboBox* combobox_filter_by_set;
  JobsMonitorFRDPlot* PlotWidget;
 
  // results
  QTableWidget* table_result;

  //storage
  QListWidgetItem* current_result_block = nullptr; //stores the current block
  QListWidgetItem* current_result_component = nullptr; //stores the current component
  QListWidgetItem* current_increment = nullptr; //stores the current inrement
  int items_per_page = 50;
  int current_page; //stores the current result page
  std::vector<int> filter;
  int results_size;
};

#endif // JOBSMONITORFRD_HPP