#ifndef JOBSMONITORLIVEMONITOR_HPP
#define JOBSMONITORLIVEMONITOR_HPP

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
#include <QComboBox>
#include <QString>
#include <QComboBox>
#include <QLineEdit>
#include <QObject>
#include <QSpacerItem>
#include <QPlainTextEdit>

class CalculiXCoreInterface;
class JobsMonitorLiveMonitorConvergence;
class JobsMonitorLiveMonitorStatus;

class JobsMonitorLiveMonitor : public QWidget
{
  Q_OBJECT
  
public:
  JobsMonitorLiveMonitor();
  ~JobsMonitorLiveMonitor();

  CalculiXCoreInterface *ccx_iface;

  int maximumBlockCount=2000;

  void clear();
  void update(QString console, std::vector<std::string> cvg, std::vector<std::string> sta);
  std::vector<std::string> split_line(std::string line);

private:
  QGridLayout* gridLayout;
  QVBoxLayout* boxLayout_windows; 
  //QLabel* label_console;
  //QPlainTextEdit* QPlainTextEdit_console;
  JobsMonitorLiveMonitorConvergence* plot_convergence;
  JobsMonitorLiveMonitorStatus* plot_status;
};

#endif // JOBSMONITORLIVEMONITOR_HPP