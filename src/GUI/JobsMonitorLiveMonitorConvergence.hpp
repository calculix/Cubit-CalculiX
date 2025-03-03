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

  QtCharts::QLineSeries *series_1;
  QtCharts::QLineSeries *series_2;
  QtCharts::QLineSeries *series_3;
  QtCharts::QLineSeries *series_4;
  QtCharts::QLineSeries *series_5;

  QtCharts::QChart *chart;
  QtCharts::QValueAxis *axisX;
  QtCharts::QLogValueAxis *axisY;

  QtCharts::QChart *chart2;
  QtCharts::QValueAxis *axisX2;
  QtCharts::QValueAxis *axisY2;
};

#endif // JOBSMONITORLIVEMONITORCONVERGENCE_HPP