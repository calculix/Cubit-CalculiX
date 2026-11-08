#include "JobsMonitorLiveMonitor.hpp"
#include <string>
#include <vector>
#include <QString>
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "JobsMonitorLiveMonitorConvergence.hpp"
#include "JobsMonitorLiveMonitorStatus.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"

#include <sstream>

JobsMonitorLiveMonitor::JobsMonitorLiveMonitor()
{

  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("Live Monitor");
  gridLayout = new QGridLayout(this);
  boxLayout_windows = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_windows,0,0, Qt::AlignLeft);
  
  plot_convergence =  new JobsMonitorLiveMonitorConvergence();
  boxLayout_windows->addWidget(plot_convergence);
  plot_status =  new JobsMonitorLiveMonitorStatus();
  boxLayout_windows->addWidget(plot_status);
  //label_console = new QLabel();
  //label_console->setText("Console Output");
  //boxLayout_windows->addWidget(label_console);
  //QPlainTextEdit_console = new QPlainTextEdit();
  //QPlainTextEdit_console->setReadOnly(true);
  //QPlainTextEdit_console->setMaximumBlockCount(maximumBlockCount);
  //boxLayout_windows->addWidget(QPlainTextEdit_console);
  
}

JobsMonitorLiveMonitor::~JobsMonitorLiveMonitor()
{}

void JobsMonitorLiveMonitor::clear()
{
  generation = 0;
  revision = 0;
  cvg_table.clear();
  sta_table.clear();
  plot_convergence->clear();
  plot_status->clear();
}

void JobsMonitorLiveMonitor::update(const CalculiXJobProgress::Data& data)
{
  if (generation == data.generation && revision == data.revision)
  {
    return;
  }
  if (generation != data.generation)
  {
    clear();
  }
  const auto previous_cvg = cvg_table.size();
  const auto previous_sta = sta_table.size();
  for (std::size_t i = previous_cvg; i < data.convergence.size(); ++i)
  {
    const auto& row = data.convergence[i];
    cvg_table.push_back({static_cast<double>(row.step), static_cast<double>(row.increment),
      static_cast<double>(row.attempt), static_cast<double>(row.iteration), static_cast<double>(row.contact_elements),
      row.measures[0], row.measures[1], row.measures[2], row.measures[3]});
  }
  for (std::size_t i = previous_sta; i < data.status.size(); ++i)
  {
    const auto& row = data.status[i];
    sta_table.push_back({static_cast<double>(row.step), static_cast<double>(row.increment),
      static_cast<double>(row.attempt), static_cast<double>(row.iterations), row.time[0], row.time[1], row.time[2]});
  }
  if (cvg_table.size() != previous_cvg)
  {
    plot_convergence->update(cvg_table);
  }
  if (sta_table.size() != previous_sta)
  {
    plot_status->update(sta_table);
  }
  generation = data.generation;
  revision = data.revision;
}
