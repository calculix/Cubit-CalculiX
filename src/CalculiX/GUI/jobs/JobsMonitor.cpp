#include "JobsMonitor.hpp"
#include <string>
#include <vector>
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/results/CalculiXResultsInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "GUI/application/GuiTimer.hpp"
#include "JobsMonitorLiveMonitor.hpp"
#include "JobsMonitorProgressTables.hpp"
#include "JobsMonitorFRD.hpp"
#include "JobsMonitorDAT.hpp"
#include "JobsMonitorDATSection.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include <QString>
#include <QStringList>
#include <QScrollBar>

JobsMonitor::JobsMonitor()
{

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


  pushButton_result_load = new QPushButton();
  pushButton_result_load->setText("Load");
  pushButton_result_load->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_result_load);
  

  pushButton_results = new QPushButton();
  pushButton_results->setText("Results");
  pushButton_results->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_results);

  pushButton_close = new QPushButton();
  pushButton_close->setText("Close");
  pushButton_close->setFixedWidth(buttonWidth);
  boxLayout_window->addWidget(pushButton_close);
  
  // textarea
  liveMonitor_widget = new JobsMonitorLiveMonitor();
  QPlainTextEdit_console = new QPlainTextEdit();
  QPlainTextEdit_console->setReadOnly(true);
  QPlainTextEdit_console->setMaximumBlockCount(maximumBlockCount);
  progress_tables = new JobsMonitorProgressTables(this);
  FRD_widget = new JobsMonitorFRD();
  DAT_widget = new JobsMonitorDAT();
  DATSection_widget = new JobsMonitorDATSection();

  //tab widget
  TabWidget = new QTabWidget();
  TabWidget->addTab(liveMonitor_widget,"Live Monitor"); 
  TabWidget->addTab(QPlainTextEdit_console,"Console Output");
  TabWidget->addTab(progress_tables->convergence_page(),"Convergence (CVG)");
  TabWidget->addTab(progress_tables->status_page(),"Status (STA)");
  TabWidget->addTab(FRD_widget,"*.frd");
  TabWidget->addTab(DAT_widget,"*.dat");
  TabWidget->addTab(DATSection_widget,"*.dat Section");
  boxLayout_tab->addWidget(TabWidget);

  // Signals
  QObject::connect(pushButton_run, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_run_clicked(bool)));
  QObject::connect(pushButton_kill, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_kill_clicked(bool)));
  QObject::connect(pushButton_result_load, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_result_load_clicked(bool)));
  QObject::connect(pushButton_results, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_results_clicked(bool)));
  QObject::connect(pushButton_close, SIGNAL(clicked(bool)),this,SLOT(on_pushButton_close_clicked(bool)));
  
  // Update list items and data
  clear();

  timer = new GuiTimer();
  timer->setParent(this);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(update_slot()));
}

JobsMonitor::~JobsMonitor()
{}

void JobsMonitor::clear()
{
  current_job_id = -1;

  total_block_count_console_output = 0;

  QPlainTextEdit_console->clear();
  progress_tables->clear();
  liveMonitor_widget->clear();

  FRD_widget->clear();
  DAT_widget->clear();
  DATSection_widget->clear();

  setWindowTitle("Jobs Monitor");

  pushButton_run->setEnabled(false);
  pushButton_kill->setEnabled(false);
  pushButton_result_load->setEnabled(false);
  pushButton_results->setEnabled(false);
}

void JobsMonitor::update_slot()
{
  this->update();
}

void JobsMonitor::update()
{
  CalculiXJobsInterface ccx_jobs_iface;

  if (current_job_id <= 0)
  {
    return;
  }

  const auto job_data = ccx_jobs_iface.get_job_data(current_job_id);

  if (job_data.empty())
  {
    clear();
    return;
  }

  const auto console_output = ccx_jobs_iface.get_job_console_output(current_job_id);
  std::string progress_error;
  const auto* progress = ccx_jobs_iface.get_job_progress(current_job_id, progress_error);

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
    bool clearLiveMonitor = false;
    // check if job has been restarted
    if (total_block_count_console_output > console_output.size())
    {
      total_block_count_console_output = 0;
      QPlainTextEdit_console->clear();
      clearLiveMonitor = true;
    }
    if (clearLiveMonitor)
    {
      liveMonitor_widget->clear();
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
    if (progress)
    {
      progress_tables->update(*progress, progress_error);
      liveMonitor_widget->update(*progress);
    }
    pushButton_run->setEnabled(true);
    pushButton_kill->setEnabled(true);
    pushButton_result_load->setEnabled(ccx_jobs_iface.get_job_status(current_job_id) != 1 &&
      !CalculiXResultsInterface().is_vtk_build_active());
    pushButton_results->setEnabled(ccx_jobs_iface.get_job_status(current_job_id) != 1 &&
      CalculiXResultsInterface().get_vtk_status(current_job_id) == SimResults::ConversionStatus::Complete);
  }else{
    this->setWindowTitle("Jobs Monitor");
    pushButton_run->setEnabled(false);
    pushButton_kill->setEnabled(false);
    pushButton_result_load->setEnabled(false);
    pushButton_results->setEnabled(false);
  }
}

void JobsMonitor::setJob(int job_id)
{
  clear();

  if (job_id <= 0)
  {
    return;
  }

  CalculiXJobsInterface ccx_jobs_iface;

  if (ccx_jobs_iface.get_job_data(job_id).empty())
  {
    return;
  }

  current_job_id = job_id;
  FRD_widget->set_current_job_id(job_id);
  DAT_widget->set_current_job_id(job_id);
  DATSection_widget->set_current_job_id(job_id);

  update();
}

void JobsMonitor::on_pushButton_run_clicked(bool)
{
  std::string command = "ccx run job " + std::to_string(current_job_id);
  GuiSendCmd::gui_cmd((QString::fromStdString(command)));
}

void JobsMonitor::on_pushButton_kill_clicked(bool)
{
  std::string command = "ccx kill job " + std::to_string(current_job_id);
  GuiSendCmd::gui_cmd((QString::fromStdString(command)));
}


void JobsMonitor::on_pushButton_result_load_clicked(bool)
{
  const std::string job = std::to_string(current_job_id);
  CalculiXApplicationInterface application;
  if (application.cmd("ccx result load job " + job))
  {
    application.cmd("ccx result convert job " + job);
  }
  update();
}


void JobsMonitor::on_pushButton_results_clicked(bool)
{
  std::string command = "post open ccx job " + std::to_string(current_job_id);
  GuiSendCmd::gui_cmd((QString::fromStdString(command)));
}

void JobsMonitor::on_pushButton_close_clicked(bool)
{
  this->close();
}

void JobsMonitor::clearJob(int job_id)
{
  if (current_job_id == job_id)
  {
    clear();
  }
}
