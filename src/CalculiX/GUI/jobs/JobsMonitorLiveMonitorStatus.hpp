#ifndef JOBSMONITORLIVEMONITORSTATUS_HPP
#define JOBSMONITORLIVEMONITORSTATUS_HPP

#include <QObject>
#include <QWidget>
#include <QString>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QList>
#include <QAbstractAxis>
#include <QValueAxis>
#include <QLegend> 
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>


class JobsMonitorLiveMonitorStatus : public QWidget
{
  Q_OBJECT
  
public:
  explicit JobsMonitorLiveMonitorStatus();
  ~JobsMonitorLiveMonitorStatus();
  
  bool isInitialized=false;

  void clear();
  void update(std::vector<std::vector<double>> sta_table);

  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_widget;

  QLineSeries *series_1;
  QLineSeries *series_2;
  QLineSeries *series_3;
  QLineSeries *series_4;
  QLineSeries *series_5;
  QLineSeries *series_6;
  QLineSeries *series_7;

  QChart *chart;
  QChart *chart2;
  QValueAxis *axisX;
  QValueAxis *axisY;
  QValueAxis *axisX2;
  QValueAxis *axisY2;
};

#endif // JOBSMONITORLIVEMONITORSTATUS_HPP