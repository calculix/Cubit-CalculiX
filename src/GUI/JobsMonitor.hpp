#ifndef JOBSMONITOR_HPP
#define JOBSMONITOR_HPP

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


class CalculiXCoreInterface;
class GUITimer;

class JobsMonitor : public QWidget
{
  Q_OBJECT
  
public:
  JobsMonitor();
  ~JobsMonitor();

  CalculiXCoreInterface *ccx_iface;
  
  void clear(); // remove all data
  void update();
  void setJob(int job_id); // sets the job to monitor
  
private slots:
  void update_slot();
  void on_pushButton_run_clicked(bool);
  void on_pushButton_kill_clicked(bool);
  void on_pushButton_result_cgx_clicked(bool);
  void on_pushButton_result_convert_clicked(bool);
  void on_pushButton_result_paraview_clicked(bool);
  void on_pushButton_close_clicked(bool);
  
private:
  std::string log;
  int current_job_id=-1;
  QTabWidget* TabWidget;
  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QHBoxLayout* boxLayout_tab;
  QPushButton* pushButton_run;
  QPushButton* pushButton_kill;
  QPushButton* pushButton_result_cgx;
  QPushButton* pushButton_result_convert;
  QPushButton* pushButton_result_paraview;
  QPushButton* pushButton_close;
  QPlainTextEdit* QPlainTextEdit_console;
  QPlainTextEdit* QPlainTextEdit_cvg;
  QPlainTextEdit* QPlainTextEdit_sta;
  GUITimer *timer;
};

#endif // JOBSMONITOR_HPP