#include "JobsMonitor.hpp"
#include "CalculiXCoreInterface.hpp"
#include "GUITimer.hpp"
#include "JobsMonitorFRD.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include <QString>
#include <QStringList>
#include <QScrollBar>

JobsMonitor::JobsMonitor()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  // main window
  this->setGeometry(0,0,800,800);
  this->setWindowTitle("Jobs Monitor");
  int buttonWidth = 133;

  gridLayout = new QGridLayout(this);
  boxLayout_window = new QHBoxLayout();
  boxLayout_tab = new QHBoxLayout();
  gridLayout->addLayout(boxLayout_window,1,0, Qt::AlignRight);
  gridLayout->addLayout(boxLayout_tab,0,0, Qt::AlignTop);
  
  // buttons
  pushButton_run = new QPushButton();
  pushButton_run->setText("Run");
  pushButton_run->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_run);

  pushButton_kill = new QPushButton();
  pushButton_kill->setText("Kill");
  pushButton_kill->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_kill);

  pushButton_result_cgx = new QPushButton();
  pushButton_result_cgx->setText("Result CGX");
  pushButton_result_cgx->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_result_cgx);

  
  pushButton_result_convert = new QPushButton();
  pushButton_result_convert->setText("convert");
  pushButton_result_convert->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_result_convert);

  pushButton_result_paraview = new QPushButton();
  pushButton_result_paraview->setText("Result ParaView");
  pushButton_result_paraview->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_result_paraview);

  pushButton_close = new QPushButton();
  pushButton_close->setText("Close");
  pushButton_close->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_close);
  
  // textarea
  QPlainTextEdit_console = new QPlainTextEdit();
  QPlainTextEdit_console->setReadOnly(true);
  QPlainTextEdit_console->setMaximumBlockCount(maximumBlockCount);
  QPlainTextEdit_cvg = new QPlainTextEdit();
  QPlainTextEdit_cvg->setReadOnly(true);
  QPlainTextEdit_cvg->setMaximumBlockCount(maximumBlockCount);
  QPlainTextEdit_sta = new QPlainTextEdit();
  QPlainTextEdit_sta->setReadOnly(true);
  QPlainTextEdit_sta->setMaximumBlockCount(maximumBlockCount);
  FRD_widget = new JobsMonitorFRD();
  DAT_widget = new QWidget();

  //tab widget
  TabWidget = new QTabWidget();
  TabWidget->addTab(QPlainTextEdit_console,"Console Output");
  TabWidget->addTab(QPlainTextEdit_cvg,"*.cvg");
  TabWidget->addTab(QPlainTextEdit_sta,"*.sta");
  TabWidget->addTab(FRD_widget,"*.frd");
  TabWidget->addTab(DAT_widget,"*.dat");
  boxLayout_tab->addWidget(TabWidget);

  // Signals
  QObject::connect(pushButton_run, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_run_clicked(bool)));
  QObject::connect(pushButton_kill, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_kill_clicked(bool)));
  QObject::connect(pushButton_result_cgx, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_result_cgx_clicked(bool)));
  QObject::connect(pushButton_result_convert, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_result_convert_clicked(bool)));
  QObject::connect(pushButton_result_paraview, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_result_paraview_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_close_clicked(bool)));
  
  // Update list items and data
  this->update();

  timer = new GUITimer();
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update_slot()));
}

JobsMonitor::~JobsMonitor()
{}

void JobsMonitor::clear()
{

}

void JobsMonitor::update_slot()
{
  this->update();
}

void JobsMonitor::update()
{
  std::vector<std::string> job_data;
  std::vector<std::string> console_output;
  std::vector<std::string> cvg;
  std::vector<std::string> sta;
  job_data = ccx_iface->get_job_data(current_job_id);
  console_output = ccx_iface->get_job_console_output(current_job_id);
  cvg = ccx_iface->get_job_cvg(current_job_id);
  sta = ccx_iface->get_job_sta(current_job_id);

  // jobs_data[0][1] name
  // jobs_data[0][2] filepath
  // jobs_data[0][3] status -1 no process, 1 process running, 2 process finished, 3 process killed, 4 process finished with errors
  // jobs_data[0][4] process id
  // jobs_data[0][5] Output Console
  // jobs_data[0][6] converted -1 false, 1 true
  // jobs_data[0][7] .cvg
  // jobs_data[0][8] .sta

  if (job_data.size()!=0)
  {
    //log = " job id " + std::to_string(current_job_id) + " \n";
    //PRINT_INFO("%s", log.c_str());

    this->setWindowTitle("Jobs Monitor - " + QString::fromStdString(job_data[1]));
    
    // check if job has been restarted
    if (total_block_count_console_output > console_output.size())
    {
      total_block_count_console_output = 0;
      QPlainTextEdit_console->clear();
    }
    if (QPlainTextEdit_cvg->blockCount() > cvg.size())
    {
      total_block_count_cvg = 0;
      QPlainTextEdit_cvg->clear();
    }
    if (QPlainTextEdit_sta->blockCount() > sta.size())
    {
      total_block_count_sta = 0;
      QPlainTextEdit_sta->clear();
    }

    if (total_block_count_console_output < console_output.size())
    {
      if (QPlainTextEdit_console->toPlainText().toStdString()=="")
      {
        QPlainTextEdit_console->appendPlainText(QString::fromStdString(console_output[0]));
        ++total_block_count_console_output;
      }
      for (size_t i = total_block_count_console_output; i < console_output.size(); i++)
      {
        ++total_block_count_console_output;
        QPlainTextEdit_console->appendPlainText(QString::fromStdString(console_output[i]));  
      }
    }
    if (QPlainTextEdit_cvg->blockCount() < cvg.size())
    {
      if (QPlainTextEdit_cvg->toPlainText().toStdString()=="")
      {
        QPlainTextEdit_cvg->appendPlainText(QString::fromStdString(cvg[0]));  
        ++total_block_count_cvg;
      }
      for (size_t i = total_block_count_cvg; i < cvg.size(); i++)
      {
        ++total_block_count_cvg;
        QPlainTextEdit_cvg->appendPlainText(QString::fromStdString(cvg[i]));  
      }
    }
    if (QPlainTextEdit_sta->blockCount() < sta.size())
    {
      if (QPlainTextEdit_sta->toPlainText().toStdString()=="")
      {
        QPlainTextEdit_sta->appendPlainText(QString::fromStdString(sta[0]));  
        ++total_block_count_sta;
      }
      for (size_t i = total_block_count_sta; i < sta.size(); i++)
      {
        ++total_block_count_sta;
        QPlainTextEdit_sta->appendPlainText(QString::fromStdString(sta[i]));  
      }
    }
    
    if (std::stoi(job_data[3])==-1)
    {
      pushButton_run->setEnabled(true);
      pushButton_kill->setEnabled(false);
      pushButton_result_cgx->setEnabled(false);
      pushButton_result_convert->setEnabled(false);
      pushButton_result_paraview->setEnabled(false);
    }else if (std::stoi(job_data[3])==1)
    {
      pushButton_run->setEnabled(false);
      pushButton_kill->setEnabled(true);
      pushButton_result_cgx->setEnabled(false);
      pushButton_result_convert->setEnabled(false);
      pushButton_result_paraview->setEnabled(false);
    }else if (std::stoi(job_data[3])>1)
    {
      pushButton_run->setEnabled(true);
      pushButton_kill->setEnabled(false);
      pushButton_result_cgx->setEnabled(true);
      if (std::stoi(job_data[6])==-1)
      {
        pushButton_result_convert->setEnabled(true);
      }else
      {
        pushButton_result_convert->setEnabled(false);
      }
      if (std::stoi(job_data[6])>0)
      {
        pushButton_result_paraview->setEnabled(true);
      }else
      {
        pushButton_result_paraview->setEnabled(false);
      }
    }
  }else{
    this->setWindowTitle("Jobs Monitor");
    pushButton_run->setEnabled(false);
    pushButton_kill->setEnabled(false);
    pushButton_result_cgx->setEnabled(false);
    pushButton_result_convert->setEnabled(false);
    pushButton_result_paraview->setEnabled(false);
  }
}

void JobsMonitor::setJob(int job_id)
{
  current_job_id = job_id;
  FRD_widget->set_current_job_id(current_job_id);
}

void JobsMonitor::on_pushButton_run_clicked(bool)
{
  std::string command = "ccx run job " + std::to_string(current_job_id);
  CubitInterface::cmd(command.c_str());
}

void JobsMonitor::on_pushButton_kill_clicked(bool)
{
  std::string command = "ccx kill job " + std::to_string(current_job_id);
  CubitInterface::cmd(command.c_str());
}

void JobsMonitor::on_pushButton_result_cgx_clicked(bool)
{
  std::string command = "ccx result cgx job " + std::to_string(current_job_id);
  CubitInterface::cmd(command.c_str());
}

void JobsMonitor::on_pushButton_result_convert_clicked(bool)
{
  std::string command = "ccx result load job " + std::to_string(current_job_id);
  CubitInterface::cmd(command.c_str());
  command = "ccx result convert job " + std::to_string(current_job_id);
  CubitInterface::cmd(command.c_str());
}

void JobsMonitor::on_pushButton_result_paraview_clicked(bool)
{
  std::string command = "ccx result paraview job " + std::to_string(current_job_id);
  CubitInterface::cmd(command.c_str());
}

void JobsMonitor::on_pushButton_close_clicked(bool)
{
  this->close();
}