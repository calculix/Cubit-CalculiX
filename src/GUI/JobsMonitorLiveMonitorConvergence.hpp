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
  /*QString windowtitle;
  QString title;
  QString x_axis;
  QString y_axis;
  std::vector<double> x_data;
  std::vector<double> y_data;*/

  void update();

  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_widget;
};

#endif // JOBSMONITORLIVEMONITORCONVERGENCE_HPP