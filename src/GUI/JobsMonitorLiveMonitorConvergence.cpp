#include "JobsMonitorLiveMonitorConvergence.hpp"

#include "CubitMessage.hpp"

JobsMonitorLiveMonitorConvergence::JobsMonitorLiveMonitorConvergence():
  isInitialized(false)
{
  if(isInitialized)
    return;

  //this->setWindowTitle(windowtitle);
  this->setGeometry(0,0,700,570);
  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_widget = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_window,1,1, Qt::AlignRight);

  series_1 = new QtCharts::QLineSeries(this);
  series_1->setPointsVisible();
  series_1->setName("FORCE");
  series_2 = new QtCharts::QLineSeries(this);
  series_2->setPointsVisible();
  series_2->setName("DISP");
  series_3 = new QtCharts::QLineSeries(this);
  series_3->setPointsVisible();
  series_3->setName("TEMP");
  chart = new QtCharts::QChart();
  chart->legend()->hide();
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignTop);
  chart->addSeries(series_1);
  chart->addSeries(series_2);
  chart->addSeries(series_3);
  //chart->createDefaultAxes();
  chart->setTitle("Convergence");
  axisX = new QtCharts::QValueAxis();
  axisX->setMin(0);
  axisX->setMax(1);
  axisX->setTickCount(4);
  axisX->setLabelFormat("%.0f");
  axisX->setTitleText("Iteration");
  series_1->attachAxis(axisX);
  series_2->attachAxis(axisX);
  series_3->attachAxis(axisX);

  
  axisY = new QtCharts::QLogValueAxis();
  //axisY->setLabelFormat("%g");
  axisY->setLabelFormat("%.4e");
  axisY->setBase(10.0);
  axisY->setMinorTickCount(-1);
  
  /*
  axisY = new QtCharts::QValueAxis();
  //axisY->setMin(1e-6);
  //axisY->setMax(1e+4);
  axisY->setTickCount(5);
  axisY->setLabelFormat("%.4e");
  */

  series_1->attachAxis(axisY);
  series_2->attachAxis(axisY);
  series_3->attachAxis(axisY);
  
  chart->addAxis(axisX,Qt::AlignBottom);
  chart->addAxis(axisY,Qt::AlignLeft);
  
  QtCharts::QChartView *chartView = new QtCharts::QChartView(chart,this);
  chartView->setRenderHint(QPainter::Antialiasing);
  
  this->setAttribute(Qt::WA_DeleteOnClose);
  boxLayout_window->addWidget(chartView);
  //std::string log;
  //log = "plotting \n";
  //PRINT_INFO("%s", log.c_str());

  isInitialized = true;
}

JobsMonitorLiveMonitorConvergence::~JobsMonitorLiveMonitorConvergence()
{
}

void JobsMonitorLiveMonitorConvergence::update(std::vector<std::vector<double>> cvg_table, std::vector<std::vector<double>> sta_table)
{
  /*
  .cvg
  0 STEP
  1 INC
  2 ATT
  3 ITER
  4 CONT EL
  5 RESID FORCE
  6 CORR DISP
  7 RESID FLUX
  8 CORR TEMP
  
  .sta
  0 STEP
  1 INC
  2 ATT
  3 ITRS
  4 TOT TIME
  5 STEP TIME
  6 INC TIME
  */
  int iter = cvg_table.size();
  if (series_1->count() < cvg_table.size())
  {
    for (size_t i = series_1->count(); i < cvg_table.size(); i++)
    {
      series_1->append(i+1,cvg_table[i][5]);
      //std::string log = "series 1 " + std::to_string(i) + " cvg_table[i][5] " + std::to_string(cvg_table[i][5]) + " \n";
      //PRINT_INFO("%s", log.c_str());
    }
  }
  
  if (series_2->count() < cvg_table.size())
  {
    for (size_t i = series_2->count(); i < cvg_table.size(); i++)
    {
     series_2->append(i+1,cvg_table[i][6]);
     //std::string log = "series 2 " + std::to_string(i) + " cvg_table[i][6] " + std::to_string(cvg_table[i][6]) + " \n";
     //PRINT_INFO("%s", log.c_str());
    }
  }
  if (series_3->count() < cvg_table.size())
  {
    for (size_t i = series_3->count(); i < cvg_table.size(); i++)
    {
     series_3->append(i+1,cvg_table[i][8]);
     //std::string log = "series 3 " + std::to_string(i) + " cvg_table[i][8] " + std::to_string(cvg_table[i][8]) + " \n";
     //PRINT_INFO("%s", log.c_str());
    }
  }

  
  double min = 0.0;
  double max = 0.0;
  for (size_t i = 0; i < cvg_table.size(); i++)
  {
    if (cvg_table[i][5] < min)
    {
      min = cvg_table[i][5];
    }
    if (cvg_table[i][5] > max)
    {
      max = cvg_table[i][5];
    }
    if (cvg_table[i][6] < min)
    {
      min = cvg_table[i][6];
    }
    if (cvg_table[i][6] > max)
    {
      max = cvg_table[i][6];
    }
    if (cvg_table[i][8] < min)
    {
      min = cvg_table[i][8];
    }
    if (cvg_table[i][8] > max)
    {
      max = cvg_table[i][8];
    }
  }
  axisX->setMax(iter);
  axisY->setMin(min);
  axisY->setMax(max);
  
  chart->removeSeries(series_1);
  chart->removeSeries(series_2);
  chart->removeSeries(series_3);
  chart->addSeries(series_1);
  chart->addSeries(series_2);
  chart->addSeries(series_3);

  //axisX->applyNiceNumbers();
  //axisY->applyNiceNumbers();
}