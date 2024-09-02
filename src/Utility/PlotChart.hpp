#ifndef PLOTCHART_HPP
#define PLOTCHART_HPP

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


class PlotChart : public QWidget
{
  Q_OBJECT
  
public:
  explicit PlotChart(QWidget *parent,QString windowtitle, QString title, QString x_axis, QString y_axis, std::vector<double> x_data,  std::vector<double> y_data, bool save_plot, QString save_filepath);
  ~PlotChart();
  
  bool isInitialized=false;
  /*QString windowtitle;
  QString title;
  QString x_axis;
  QString y_axis;
  std::vector<double> x_data;
  std::vector<double> y_data;*/

  QGridLayout* gridLayout;
  QHBoxLayout* boxLayout_window;
  QVBoxLayout* boxLayout_widget;
};

#endif // PLOTCHART_HPP