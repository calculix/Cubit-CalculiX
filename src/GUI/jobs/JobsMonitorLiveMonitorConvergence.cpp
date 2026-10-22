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

  series_1 = new QLineSeries(this);
  series_1->setPointsVisible(false);
  series_1->setName("FORCE");
  series_2 = new QLineSeries(this);
  series_2->setPointsVisible(false);
  series_2->setName("DISP");
  series_3 = new QLineSeries(this);
  series_3->setPointsVisible(false);
  series_3->setName("FLUX");
  series_4 = new QLineSeries(this);
  series_4->setPointsVisible(false);
  series_4->setName("TEMP");
  series_5 = new QLineSeries(this);
  series_5->setPointsVisible(false);
  series_5->setName("Contact Elements");
  chart = new QChart();
  //chart->legend()->hide();
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignTop);
  chart->addSeries(series_1);
  chart->addSeries(series_2);
  chart->addSeries(series_3);
  chart->addSeries(series_4);
  //chart->createDefaultAxes();
  //chart->setTitle("Convergence Information");
  chart2 = new QChart();
  //chart2->legend()->hide();
  chart2->legend()->setVisible(true);
  chart2->legend()->setAlignment(Qt::AlignTop);
  chart2->addSeries(series_5);
  axisX = new QValueAxis();
  axisX->setRange(1,1);
  axisX->setMin(1);
  axisX->setMax(1);
  axisX->setLabelFormat("%u");
  axisX->setTitleText("Iteration");
  axisX2 = new QValueAxis();
  axisX2->setRange(1,1);
  axisX2->setMin(1);
  axisX2->setMax(1);
  axisX2->setLabelFormat("%u");
  axisX2->setTitleText("Iteration");
  series_1->attachAxis(axisX);
  series_2->attachAxis(axisX);
  series_3->attachAxis(axisX);
  series_4->attachAxis(axisX);
  series_5->attachAxis(axisX2);
  
  axisY = new QLogValueAxis();
  //axisY->setLabelFormat("%g");
  axisY->setTitleText("%");
  axisY->setLabelFormat("%.0e");
  axisY->setBase(10.0);
  axisY->setMin(0);
  axisY->setMax(0);
  axisY->setMinorTickCount(0);
  
  axisY2 = new QValueAxis();
  axisY2->setMin(0);
  axisY2->setMax(0);
  axisY2->setTitleText("#");
  //axisY2->setTickCount(5);
  axisY2->setLabelFormat("%u");

  series_1->attachAxis(axisY);
  series_2->attachAxis(axisY);
  series_3->attachAxis(axisY);
  series_4->attachAxis(axisY);
  series_5->attachAxis(axisY2);
  
  chart->addAxis(axisX,Qt::AlignBottom);
  chart->addAxis(axisY,Qt::AlignLeft);
  chart2->addAxis(axisX2,Qt::AlignBottom);
  chart2->addAxis(axisY2,Qt::AlignLeft);
  
  QChartView *chartView = new QChartView(chart,this);
  chartView->setRenderHint(QPainter::Antialiasing);
  QChartView *chartView2 = new QChartView(chart2,this);
  chartView2->setRenderHint(QPainter::Antialiasing);
  
  this->setAttribute(Qt::WA_DeleteOnClose);
  boxLayout_window->addWidget(chartView);
  boxLayout_window->addWidget(chartView2);
  //std::string log;
  //log = "plotting \n";
  //PRINT_INFO("%s", log.c_str());

  isInitialized = true;
}

JobsMonitorLiveMonitorConvergence::~JobsMonitorLiveMonitorConvergence()
{
}

void JobsMonitorLiveMonitorConvergence::clear()
{
  series_1->clear();
  series_2->clear();
  series_3->clear();
  series_4->clear();
  series_5->clear();
  axisX->setRange(1,1);
  axisX->setMin(1);
  axisX->setMax(1);
  axisX2->setRange(1,1);
  axisX2->setMin(1);
  axisX2->setMax(1);
  axisY->setMin(0);
  axisY->setMax(0);
  axisY2->setMin(0);
  axisY2->setMax(0);
  chart->removeSeries(series_1);
  chart->removeSeries(series_2);
  chart->removeSeries(series_3);
  chart->removeSeries(series_4);
  chart2->removeSeries(series_5);
  chart->addSeries(series_1);
  chart->addSeries(series_2);
  chart->addSeries(series_3);
  chart->addSeries(series_4);
  chart2->addSeries(series_5);
  series_1->attachAxis(axisX);
  series_2->attachAxis(axisX);
  series_3->attachAxis(axisX);
  series_4->attachAxis(axisX);
  series_5->attachAxis(axisX2);
  series_1->attachAxis(axisY);
  series_2->attachAxis(axisY);
  series_3->attachAxis(axisY);
  series_4->attachAxis(axisY);
  series_5->attachAxis(axisY2);
}

void JobsMonitorLiveMonitorConvergence::update(std::vector<std::vector<double>> cvg_table)
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
  */
  bool redraw = false;
  int iter = cvg_table.size();
  double minValue = 1e-6;
  if (series_1->count() < cvg_table.size())
  {
    for (size_t i = series_1->count(); i < cvg_table.size(); i++)
    {
      if (cvg_table[i][5]==0.0)
      {
        series_1->append(i+1,minValue);
      }else{
        series_1->append(i+1,cvg_table[i][5]);
      }
      if (cvg_table[i][6]==0.0)
      {
        series_2->append(i+1,minValue);
      }else{
        series_2->append(i+1,cvg_table[i][6]);
      }
      if (cvg_table[i][7]==0.0)
      {
        series_3->append(i+1,minValue);
      }else{
        series_3->append(i+1,cvg_table[i][7]);
      }
      if (cvg_table[i][8]==0.0)
      {
        series_4->append(i+1,minValue);
      }else{
        series_4->append(i+1,cvg_table[i][8]);
      }
      
      series_5->append(i+1,cvg_table[i][4]);
      //std::string log = "series 1 " + std::to_string(i) + " cvg_table[i][5] " + std::to_string(cvg_table[i][5]) + " \n";
      //PRINT_INFO("%s", log.c_str());
    }
    redraw = true;
  }
  
  if (redraw)
  {
    double min = 0.0;
    double max = 0.0;
    for (size_t i = 0; i < cvg_table.size(); i++)
    {
      for (size_t ii = 5; ii < 9; ii++)
      {
        if (cvg_table[i][ii] < min)
        {
          min = cvg_table[i][ii];
        }
        if (cvg_table[i][ii] > max)
        {
          max = cvg_table[i][ii];
        }
      }
    }
    axisX->setRange(1,iter);
    axisX->setMax(iter);
    if (min==0.0)
    {
      min = minValue;
    }
    axisY->setMin(min);
    axisY->setMax(max);
    
    min = 0.0;
    max = 0.0;
    for (size_t i = 0; i < cvg_table.size(); i++)
    {
      if (cvg_table[i][4] < min)
      {
        min = cvg_table[i][4];
      }
      if (cvg_table[i][4] > max)
      {
        max = cvg_table[i][4];
      }
    }
    axisX2->setRange(1,iter);
    axisX2->setMax(iter);
    axisY2->setMin(min);
    axisY2->setMax(max);
    
    chart->removeSeries(series_1);
    chart->removeSeries(series_2);
    chart->removeSeries(series_3);
    chart->removeSeries(series_4);
    chart2->removeSeries(series_5);
    chart->addSeries(series_1);
    chart->addSeries(series_2);
    chart->addSeries(series_3);
    chart->addSeries(series_4);
    chart2->addSeries(series_5);
 
    series_1->attachAxis(axisX);
    series_2->attachAxis(axisX);
    series_3->attachAxis(axisX);
    series_4->attachAxis(axisX);
    series_5->attachAxis(axisX2);
    series_1->attachAxis(axisY);
    series_2->attachAxis(axisY);
    series_3->attachAxis(axisY);
    series_4->attachAxis(axisY);
    series_5->attachAxis(axisY2);
    //axisX->applyNiceNumbers();
    //axisY->applyNiceNumbers();
  }
}