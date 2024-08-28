#ifndef PLOTCHART_HPP
#define PLOTCHART_HPP

#include <QObject>
#include <QWidget>
#include <QString>
#include <QChart>
#include <QChartView>
#include <QLineSeries>


class PlotChart : public QWidget
{
  Q_OBJECT
  
public:
  explicit PlotChart(QWidget *parent);
  ~PlotChart();
  
  bool isInitialized=false;
//private slots:
  
//private: 

};

#endif // PLOTCHART_HPP