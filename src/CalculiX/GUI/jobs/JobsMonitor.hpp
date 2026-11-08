#ifndef JOBSMONITOR_HPP
#define JOBSMONITOR_HPP
#include <string>

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
#include <QPlainTextEdit>
#include <QTabWidget>
class GuiTimer;
class JobsMonitorLiveMonitor;
class JobsMonitorProgressTables;
class JobsMonitorFRD;
class JobsMonitorDAT;
class JobsMonitorDATSection;

class JobsMonitor : public QWidget
{
  Q_OBJECT
  
public:
  JobsMonitor();
  ~JobsMonitor();

  
  void clearJob(int job_id);
  void clear(); // remove all data
  void update();
  void setJob(int job_id); // sets the job to monitor
  
private slots:
  void update_slot();
  void on_pushButton_run_clicked(bool);
  void on_pushButton_kill_clicked(bool);
  void on_pushButton_result_load_clicked(bool);
  void on_pushButton_results_clicked(bool);
  void on_pushButton_close_clicked(bool);
  
private:
  std::string log;
  int current_job_id=-1;
  int maximumBlockCount=2000;
  int total_block_count_console_output = 0;
  QTabWidget* TabWidget;
  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QHBoxLayout* boxLayout_tab;
  QPushButton* pushButton_run;
  QPushButton* pushButton_kill;
  QPushButton* pushButton_result_load;
  QPushButton* pushButton_results;
  QPushButton* pushButton_close;
  JobsMonitorLiveMonitor* liveMonitor_widget;
  JobsMonitorProgressTables* progress_tables;
  QPlainTextEdit* QPlainTextEdit_console;
  JobsMonitorFRD* FRD_widget;
  JobsMonitorDAT* DAT_widget;
  JobsMonitorDATSection* DATSection_widget;

  GuiTimer *timer;
};

#endif // JOBSMONITOR_HPP