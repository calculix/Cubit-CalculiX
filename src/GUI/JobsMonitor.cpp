#include "JobsMonitor.hpp"
#include "CalculiXCoreInterface.hpp"
#include "GUITimer.hpp"

#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"
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
  QTextEdit_console = new QTextEdit();
  QTextEdit_console->setReadOnly(true);
  QTextEdit_cvg = new QTextEdit();
  QTextEdit_cvg->setReadOnly(true);
  QTextEdit_sta = new QTextEdit();
  QTextEdit_sta->setReadOnly(true);

  //tab widget
  TabWidget = new QTabWidget();
  TabWidget->addTab(QTextEdit_console,"Console Output");
  TabWidget->addTab(QTextEdit_cvg,"*.cvg");
  TabWidget->addTab(QTextEdit_sta,"*.sta");
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
  job_data = ccx_iface->get_job_data(current_job_id);

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
    
    if (QTextEdit_console->toPlainText()!=QString::fromStdString(job_data[5]))
    {
      QTextEdit_console->setText(QString::fromStdString(job_data[5]));
      QTextEdit_console->verticalScrollBar()->setValue(QTextEdit_console->verticalScrollBar()->maximum());
    }
    if (QTextEdit_cvg->toPlainText()!=QString::fromStdString(job_data[7]))
    {
      QTextEdit_cvg->setText(QString::fromStdString(job_data[7]));
      QTextEdit_cvg->verticalScrollBar()->setValue(QTextEdit_cvg->verticalScrollBar()->maximum());
    }
    if (QTextEdit_sta->toPlainText()!=QString::fromStdString(job_data[8]))
    {
      QTextEdit_sta->setText(QString::fromStdString(job_data[8]));
      QTextEdit_sta->verticalScrollBar()->setValue(QTextEdit_sta->verticalScrollBar()->maximum());
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
}

void JobsMonitor::on_pushButton_run_clicked(bool)
{
  QStringList commands;
  QString command = "";

  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    command = "ccx run job " + QString::number(current_job_id);
    commands.push_back(command);
    for(int i = 0; i < commands.size(); i++)
    cubit_translator->decode(commands[i]);
    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void JobsMonitor::on_pushButton_kill_clicked(bool)
{
  QStringList commands;
  QString command = "";

  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    command = "ccx kill job " + QString::number(current_job_id);
    commands.push_back(command);
    for(int i = 0; i < commands.size(); i++)
    cubit_translator->decode(commands[i]);
    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void JobsMonitor::on_pushButton_result_cgx_clicked(bool)
{
  QStringList commands;
  QString command = "";

  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    command = "ccx result cgx job " + QString::number(current_job_id);
    commands.push_back(command);
    for(int i = 0; i < commands.size(); i++)
    cubit_translator->decode(commands[i]);
      // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void JobsMonitor::on_pushButton_result_convert_clicked(bool)
{
  QStringList commands;
  QString command = "";

  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    command = "ccx result load job " + QString::number(current_job_id);
    commands.push_back(command);
    command = "ccx result convert job " + QString::number(current_job_id);
    commands.push_back(command);
    for(int i = 0; i < commands.size(); i++)
    cubit_translator->decode(commands[i]);
    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void JobsMonitor::on_pushButton_result_paraview_clicked(bool)
{
  QStringList commands;
  QString command = "";

  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    command = "ccx result paraview job " + QString::number(current_job_id);
    commands.push_back(command);
    for(int i = 0; i < commands.size(); i++)
    cubit_translator->decode(commands[i]);
    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void JobsMonitor::on_pushButton_close_clicked(bool)
{
  this->close();
}