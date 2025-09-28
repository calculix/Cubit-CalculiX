#include "JobsMonitorLiveMonitorStatus.hpp"

#include "CubitMessage.hpp"

JobsMonitorLiveMonitorStatus::JobsMonitorLiveMonitorStatus():
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
  series_1->setName("STEP");
  series_2 = new QLineSeries(this);
  series_2->setPointsVisible(false);
  series_2->setName("INC");
  series_3 = new QLineSeries(this);
  series_3->setPointsVisible(false);
  series_3->setName("ATT");
  series_4 = new QLineSeries(this);
  series_4->setPointsVisible(false);
  series_4->setName("ITRS");
  series_5 = new QLineSeries(this);
  series_5->setPointsVisible(false);
  series_5->setName("TOT TIME");
  series_6 = new QLineSeries(this);
  series_6->setPointsVisible(false);
  series_6->setName("STEP TIME");
  series_7 = new QLineSeries(this);
  series_7->setPointsVisible(false);
  series_7->setName("INC TIME");
  chart = new QChart();
  //chart->legend()->hide();
  chart->legend()->setVisible(true);
  chart->legend()->setAlignment(Qt::AlignTop);
  //chart->createDefaultAxes();
  //chart->setTitle("Status Information");
  chart2 = new QChart();
  //chart2->legend()->hide();
  chart2->legend()->setVisible(true);
  chart2->legend()->setAlignment(Qt::AlignTop);

  axisX = new QValueAxis();
  axisX->setRange(1,1);
  axisX->setMin(1);
  axisX->setMax(1);
  //axisX->setTickCount(4);
  axisX->setLabelFormat("%u");
  axisX->setTitleText("Increment");
  axisX2 = new QValueAxis();
  axisX2->setRange(1,1);
  axisX2->setMin(1);
  axisX2->setMax(1);
  //axisX2->setTickCount(4);
  axisX2->setLabelFormat("%u");
  axisX2->setTitleText("Increment");
  
  axisY = new QValueAxis();
  axisY->setMin(0);
  axisY->setMax(0);
  //axisY->setTickCount(5);
  axisY->setLabelFormat("%u");
  axisY->setTitleText("");
  
  axisY2 = new QValueAxis();
  axisY2->setMin(0);
  axisY2->setMax(0);
  //axisY2->setTickCount(5);
  axisY2->setLabelFormat("%.2e");
  axisY2->setTitleText("Time");
  
  chart->addSeries(series_1);
  chart->addSeries(series_2);
  chart->addSeries(series_3);
  chart->addSeries(series_4);
  chart2->addSeries(series_5);
  chart2->addSeries(series_6);
  chart2->addSeries(series_7);
  chart->addAxis(axisX,Qt::AlignBottom);
  chart->addAxis(axisY,Qt::AlignLeft);
  chart2->addAxis(axisX2,Qt::AlignBottom);
  chart2->addAxis(axisY2,Qt::AlignLeft);
  series_1->attachAxis(axisX);
  series_2->attachAxis(axisX);
  series_3->attachAxis(axisX);
  series_4->attachAxis(axisX);
  series_5->attachAxis(axisX2);
  series_6->attachAxis(axisX2);
  series_7->attachAxis(axisX2);  
  series_1->attachAxis(axisY);
  series_2->attachAxis(axisY);
  series_3->attachAxis(axisY);
  series_4->attachAxis(axisY);
  series_5->attachAxis(axisY2);
  series_6->attachAxis(axisY2);
  series_7->attachAxis(axisY2);
  
  
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

JobsMonitorLiveMonitorStatus::~JobsMonitorLiveMonitorStatus()
{
}

void JobsMonitorLiveMonitorStatus::clear()
{
  series_1->clear();
  series_2->clear();
  series_3->clear();
  series_4->clear();
  series_5->clear();
  series_6->clear();
  series_7->clear();
  axisX->setRange(1,1);
  axisX->setMin(1);
  axisX->setMax(1);
  axisX2->setRange(1,1);
  axisX2->setMin(1);
  axisX2->setMax(1);
  axisY->setRange(0,0);
  axisY->setMin(0);
  axisY->setMax(0);
  axisY2->setRange(0,0);
  axisY2->setMin(0);
  axisY2->setMax(0);
  chart->removeSeries(series_1);
  chart->removeSeries(series_2);
  chart->removeSeries(series_3);
  chart->removeSeries(series_4);
  chart2->removeSeries(series_5);
  chart2->removeSeries(series_6);
  chart2->removeSeries(series_7);
  chart->addSeries(series_1);
  chart->addSeries(series_2);
  chart->addSeries(series_3);
  chart->addSeries(series_4);
  chart2->addSeries(series_5);
  chart2->addSeries(series_6);
  chart2->addSeries(series_7);
  series_1->attachAxis(axisX);
  series_2->attachAxis(axisX);
  series_3->attachAxis(axisX);
  series_4->attachAxis(axisX);
  series_5->attachAxis(axisX2);
  series_6->attachAxis(axisX2);
  series_7->attachAxis(axisX2);  
  series_1->attachAxis(axisY);
  series_2->attachAxis(axisY);
  series_3->attachAxis(axisY);
  series_4->attachAxis(axisY);
  series_5->attachAxis(axisY2);
  series_6->attachAxis(axisY2);
  series_7->attachAxis(axisY2);
}

void JobsMonitorLiveMonitorStatus::update(std::vector<std::vector<double>> sta_table)
{
  /*
  .sta
  0 STEP
  1 INC
  2 ATT
  3 ITRS
  4 TOT TIME
  5 STEP TIME
  6 INC TIME
  */
  bool redraw = false;
  int inc = sta_table.size();
  if (series_1->count() < sta_table.size())
  {
    for (size_t i = series_1->count(); i < sta_table.size(); i++)
    {
      series_1->append(i+1,sta_table[i][0]);
      series_2->append(i+1,sta_table[i][1]);
      series_3->append(i+1,sta_table[i][2]);
      series_4->append(i+1,sta_table[i][3]);
      series_5->append(i+1,sta_table[i][4]);
      series_6->append(i+1,sta_table[i][5]);
      series_7->append(i+1,sta_table[i][6]);
      //std::string log = "series 1 " + std::to_string(i) + " sta_table[i][1] " + std::to_string(sta_table[i][1]) + " \n";
      //PRINT_INFO("%s", log.c_str());
    }
    redraw = true;
  }
  
  if (redraw)
  {
    double min = 0.0;
    double max = 0.0;
    for (size_t i = 0; i < sta_table.size(); i++)
    {
      for (size_t ii = 0; ii < 4; ii++)
      {
        if (sta_table[i][ii] < min)
        {
          min = sta_table[i][ii];
        }
        if (sta_table[i][ii] > max)
        {
          max = sta_table[i][ii];
        }
      }
    }
    axisX->setRange(1,inc);
    axisX->setMax(inc);
    axisY->setRange(min,max);
    axisY->setMin(min);
    axisY->setMax(max);
    
    min = 0.0;
    max = 0.0;
    for (size_t i = 0; i < sta_table.size(); i++)
    {
      for (size_t ii = 4; ii < 7; ii++)
      {
        if (sta_table[i][ii] < min)
        {
          min = sta_table[i][ii];
        }
        if (sta_table[i][ii] > max)
        {
          max = sta_table[i][ii];
        }
      }
    }
    axisX2->setRange(1,inc);
    axisX2->setMax(inc);
    axisY2->setRange(min,max); 
    axisY2->setMin(min);
    axisY2->setMax(max);

    chart->removeSeries(series_1);
    chart->removeSeries(series_2);
    chart->removeSeries(series_3);
    chart->removeSeries(series_4);
    chart2->removeSeries(series_5);
    chart2->removeSeries(series_6);
    chart2->removeSeries(series_7);
    chart->addSeries(series_1);
    chart->addSeries(series_2);
    chart->addSeries(series_3);
    chart->addSeries(series_4);
    chart2->addSeries(series_5);
    chart2->addSeries(series_6);
    chart2->addSeries(series_7);
  
    series_1->attachAxis(axisX);
    series_2->attachAxis(axisX);
    series_3->attachAxis(axisX);
    series_4->attachAxis(axisX);
    series_5->attachAxis(axisX2);
    series_6->attachAxis(axisX2);
    series_7->attachAxis(axisX2);  
    series_1->attachAxis(axisY);
    series_2->attachAxis(axisY);
    series_3->attachAxis(axisY);
    series_4->attachAxis(axisY);
    series_5->attachAxis(axisY2);
    series_6->attachAxis(axisY2);
    series_7->attachAxis(axisY2);        
  }
}