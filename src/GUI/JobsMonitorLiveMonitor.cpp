#include "JobsMonitorLiveMonitor.hpp"
#include "CalculiXCoreInterface.hpp"
#include "JobsMonitorLiveMonitorConvergence.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"

JobsMonitorLiveMonitor::JobsMonitorLiveMonitor()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  //this->setGeometry(0,0,700,570);
  this->setWindowTitle("Live Monitor");
  int comboBoxWidth = 120;
  gridLayout = new QGridLayout(this);
  boxLayout_windows = new QVBoxLayout();
  gridLayout->addLayout(boxLayout_windows,0,0, Qt::AlignLeft);
  
  plot_convergence =  new JobsMonitorLiveMonitorConvergence();
  boxLayout_windows->addWidget(plot_convergence);
  plot_status =  new JobsMonitorLiveMonitorConvergence();
  boxLayout_windows->addWidget(plot_status);
  label_console = new QLabel();
  label_console->setText("Console Output");
  boxLayout_windows->addWidget(label_console);
  QPlainTextEdit_console = new QPlainTextEdit();
  QPlainTextEdit_console->setReadOnly(true);
  QPlainTextEdit_console->setMaximumBlockCount(maximumBlockCount);
  boxLayout_windows->addWidget(QPlainTextEdit_console);
  
}

JobsMonitorLiveMonitor::~JobsMonitorLiveMonitor()
{}

void JobsMonitorLiveMonitor::clear()
{
}

void JobsMonitorLiveMonitor::update(QString console, std::vector<std::string> cvg,std::vector<std::string> sta)
{
  //update console
  QPlainTextEdit_console->clear();
  QPlainTextEdit_console->appendPlainText(console);
 
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

  std::vector<std::vector<double>> cvg_table;
  if (cvg.size()>5)
  {
    for (size_t i = 5; i < cvg.size(); i++)
    {
      std::vector<std::string> line = this->split_line(cvg[i]);
      
      std::string log = "i " + std::to_string(i) + " line[0] " + line[0] + " \n";
      PRINT_INFO("%s", log.c_str());
    }
    
  }
  
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

  plot_convergence->update();
}

std::vector<std::string> JobsMonitorLiveMonitor::split_line(std::string line)
{
  std::vector<std::string> str_array;
  std::string str_temp;

  // trim whitespaces
  size_t strBegin = line.find_first_not_of(" \t\r\f\v\n");
  if (strBegin != std::string::npos)
  {
    size_t strEnd = line.find_last_not_of(" \t\r\f\v\n");
    size_t strRange = strEnd - strBegin + 1;
    line = line.substr(strBegin, strRange);
  }
  // reduce whitespaces
  size_t beginSpace = line.find_first_of(" \t\r\f\v\n");
  while (beginSpace != std::string::npos)
  {
    size_t endSpace = line.find_first_not_of(" \t\r\f\v\n", beginSpace);
    size_t range = endSpace - beginSpace;

    line.replace(beginSpace,range, " ");
    size_t newStart = beginSpace + 1;
    beginSpace = line.find_first_of(" \t\r\f\v\n",newStart);
  }
  // insert whitespaces before "-" whitespaces but not "E-"
  beginSpace = line.find_first_of("-");
  while (beginSpace != std::string::npos)
  {
    size_t range = 1;
    if ((line[beginSpace-1] != 'E') && (line[beginSpace-1] != ' ') && (beginSpace !=0))
    {
      line.replace(beginSpace,range, " -");
    }
    size_t newStart = beginSpace + 2;
    beginSpace = line.find_first_of("-",newStart);
  }
  // insert whitespaces after "set" if there is none
  beginSpace = line.find("set");
  if (beginSpace != std::string::npos)
  {
    size_t range = 3;
    if (line[beginSpace+3] != ' ')
    {
      line.replace(beginSpace,range, "set ");
    }
  }
  // insert whitespaces before "for" if there is none
  beginSpace = line.find("for");
  if ((beginSpace != std::string::npos) && (beginSpace != 0))
  {
    size_t range = 3;
    if (line[beginSpace-1] != ' ')
    {
      line.replace(beginSpace,range, " for");
    }
  }

  std::stringstream ss(line);

  // split line with delimiter " "
  while (getline(ss,str_temp,' '))
  {
    str_array.push_back(str_temp);
  }
  
  return str_array;
}