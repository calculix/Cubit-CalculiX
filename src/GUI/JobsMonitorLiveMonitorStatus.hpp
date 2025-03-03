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

  QtCharts::QLineSeries *series_1;
  QtCharts::QLineSeries *series_2;
  QtCharts::QLineSeries *series_3;
  QtCharts::QLineSeries *series_4;
  QtCharts::QLineSeries *series_5;
  QtCharts::QLineSeries *series_6;
  QtCharts::QLineSeries *series_7;

  QtCharts::QChart *chart;
  QtCharts::QChart *chart2;
  QtCharts::QValueAxis *axisX;
  QtCharts::QValueAxis *axisY;
  QtCharts::QValueAxis *axisX2;
  QtCharts::QValueAxis *axisY2;
};

#endif // JOBSMONITORLIVEMONITORSTATUS_HPP