#include "PlotChart.hpp"

#include "CubitMessage.hpp"

PlotChart::PlotChart(QWidget* parent,QString windowtitle, QString title, QString x_axis, QString y_axis, std::vector<double> x_data,  std::vector<double> y_data,bool save_plot, QString save_filepath):
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;

  this->setWindowTitle(windowtitle);
  this->setGeometry(0,0,700,570);
  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,1,1, Qt::AlignRight);

  QtCharts::QLineSeries *series = new QtCharts::QLineSeries(this);
  series->setPointsVisible();
  for (size_t i = 0; i < x_data.size(); i++)
  {
    series->append(x_data[i], y_data[i]);
  }

  QtCharts::QChart *chart = new QtCharts::QChart();
  chart->legend()->hide();
  chart->addSeries(series);
  chart->createDefaultAxes();
  chart->setTitle(title);
  QList<QtCharts::QAbstractAxis*> axes = chart->axes();
  axes[0]->setTitleText(x_axis);
  axes[1]->setTitleText(y_axis);

  QtCharts::QChartView *chartView = new QtCharts::QChartView(chart,this);
  chartView->setRenderHint(QPainter::Antialiasing);
  
  this->setAttribute(Qt::WA_DeleteOnClose);
  boxLayout_window->addWidget(chartView);

  if (save_plot)
  {
    chartView->grab().save(save_filepath);
  }

  //std::string log;
  //log = "plotting \n";
  //PRINT_INFO("%s", log.c_str());

  isInitialized = true;
}

PlotChart::~PlotChart()
{
  //std::string log;
  //log = "closing plot for job \n";
  //PRINT_INFO("%s", log.c_str());
}