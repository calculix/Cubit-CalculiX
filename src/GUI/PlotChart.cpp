#include "PlotChart.hpp"

#include "CubitMessage.hpp"

PlotChart::PlotChart(QWidget* parent):
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;

  QtCharts::QLineSeries *series = new QtCharts::QLineSeries(this);
  series->append(0, 6);
  series->append(2, 4);
  series->append(3, 8);
  series->append(7, 4);
  series->append(10, 5);
  *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

  QtCharts::QChart *chart = new QtCharts::QChart();
  chart->legend()->hide();
  chart->addSeries(series);
  chart->createDefaultAxes();
  chart->setTitle("Simple line chart example");

  QtCharts::QChartView *chartView = new QtCharts::QChartView(chart,this);
  chartView->setRenderHint(QPainter::Antialiasing);
  
  //chartView->show();
  //chartView->setAttribute(Qt::WA_DeleteOnClose);

  std::string log;
  log = "plotting \n";
  PRINT_INFO("%s", log.c_str());

  isInitialized = true;
}

PlotChart::~PlotChart()
{
  std::string log;
  log = "closing plot for job \n";
  PRINT_INFO("%s", log.c_str());
}