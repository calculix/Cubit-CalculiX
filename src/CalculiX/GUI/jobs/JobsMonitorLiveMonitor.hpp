#ifndef JOBSMONITORLIVEMONITOR_HPP
#define JOBSMONITORLIVEMONITOR_HPP
#include "Core/jobs/JobProgress.hpp"
#include <string>
#include <vector>

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
class JobsMonitorLiveMonitorConvergence;
class JobsMonitorLiveMonitorStatus;

class JobsMonitorLiveMonitor : public QWidget
{
  Q_OBJECT
  
public:
  JobsMonitorLiveMonitor();
  ~JobsMonitorLiveMonitor();


  int maximumBlockCount=2000;

  void clear();
  void update(const CalculiXJobProgress::Data& data);

private:
  std::uint64_t generation = 0;
  std::uint64_t revision = 0;
  std::vector<std::vector<double>> cvg_table;
  std::vector<std::vector<double>> sta_table;
  QGridLayout* gridLayout;
  QVBoxLayout* boxLayout_windows; 
  //QLabel* label_console;
  //QPlainTextEdit* QPlainTextEdit_console;
  JobsMonitorLiveMonitorConvergence* plot_convergence;
  JobsMonitorLiveMonitorStatus* plot_status;
};

#endif // JOBSMONITORLIVEMONITOR_HPP