#ifndef JOBSMONITORDATSECTION_HPP
#define JOBSMONITORDATSECTION_HPP

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
class JobsMonitorDATSectionPlot;
class PickWidget;

class JobsMonitorDATSection : public QWidget
{
  Q_OBJECT
  
public:
  JobsMonitorDATSection();
  ~JobsMonitorDATSection();

  CalculiXCoreInterface *ccx_iface;

  int current_job_id = -1;
  
  void clear(); // remove all data
  void update();
  void update_time();
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
  void result_set_clicked(QListWidgetItem* item);
  void result_set_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item);
  void time_clicked(QListWidgetItem* item);
  void time_changed(QListWidgetItem* current_item, QListWidgetItem* prev_item);

private:
  std::string log;
  QGridLayout* gridLayout;
  QVBoxLayout* boxLayout_result_set;
  QVBoxLayout* boxLayout_time;
  QHBoxLayout* boxLayout_result_buttons;
  QVBoxLayout* boxLayout_widget;
  QHBoxLayout* boxLayout_pages;
  QPushButton* pushButton_reset;
  QPushButton* pushButton_plot;
  QPushButton* pushButton_query_results;
  QPushButton* pushButton_prev;
  QPushButton* pushButton_next;
  QPushButton* pushButton_export;
  QLabel* label_result_set;
  QLabel* label_time;
  QListWidget* list_result_set;
  QListWidget* list_time;
  QTableWidget* table_time;
  QLabel* table_counter;  
  QComboBox* combobox_filter_by_set;
  JobsMonitorDATSectionPlot* PlotWidget;
 
  // results
  QTableWidget* table_result;

  //storage
  QListWidgetItem* current_result_set = nullptr; //stores the current set
  QListWidgetItem* current_time = nullptr; //stores the current inrement
  int items_per_page = 50;
  int current_page; //stores the current result page
  std::vector<int> filter;
  int results_size;
};

#endif // JOBSMONITORDATSECTION_HPP