#ifndef JOBSMONITORLIVEMONITORCONVERGENCE_HPP
#define JOBSMONITORLIVEMONITORCONVERGENCE_HPP

#include <QObject>
#include <QWidget>
#include <QString>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QList>
#include <QAbstractAxis>
#include <QValueAxis>
#include <QLogValueAxis>
#include <QLegend> 
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>


class JobsMonitorLiveMonitorConvergence : public QWidget
{
  Q_OBJECT
  
public:
  explicit JobsMonitorLiveMonitorConvergence();
  ~JobsMonitorLiveMonitorConvergence();
  
  bool isInitialized=false;

  void clear();
  void update(std::vector<std::vector<double>> cvg_table);

  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_widget;

  QLineSeries *series_1;
  QLineSeries *series_2;
  QLineSeries *series_3;
  QLineSeries *series_4;
  QLineSeries *series_5;

  QChart *chart;
  QValueAxis *axisX;
  QLogValueAxis *axisY;

  QChart *chart2;
  QValueAxis *axisX2;
  QValueAxis *axisY2;
};

#endif // JOBSMONITORLIVEMONITORCONVERGENCE_HPP