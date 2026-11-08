#include "JobsActionsPanel.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


JobsActionsPanel::JobsActionsPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
  int labelWidth = 120;
  int buttonWidth = 200;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  label_1 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_1->setText("Job ID");
  lineEdit_1 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  pushButton_run = new QPushButton();
  pushButton_run->setText("Run");
  pushButton_run->setFixedWidth(buttonWidth);
  HBoxLayout_pushButton_run = new QHBoxLayout();
  horizontal_spacer_pushButton_run = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  pushButton_kill = new QPushButton();
  pushButton_kill->setText("Kill");
  pushButton_kill->setFixedWidth(buttonWidth);
  HBoxLayout_pushButton_kill = new QHBoxLayout();
  horizontal_spacer_pushButton_kill = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  pushButton_result_load = new QPushButton();
  pushButton_result_load->setText("Load");
  pushButton_result_load->setFixedWidth(buttonWidth);
  HBoxLayout_pushButton_result_load = new QHBoxLayout();
  horizontal_spacer_pushButton_result_load = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  pushButton_results = new QPushButton();
  pushButton_results->setText("Results");
  pushButton_results->setFixedWidth(buttonWidth);
  HBoxLayout_pushButton_results = new QHBoxLayout();
  horizontal_spacer_pushButton_results = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  
  VBoxLayout->addLayout(HBoxLayout_pushButton_run);
  VBoxLayout->addLayout(HBoxLayout_pushButton_kill);
  VBoxLayout->addLayout(HBoxLayout_pushButton_result_load);
  VBoxLayout->addLayout(HBoxLayout_pushButton_results);
  VBoxLayout->addItem(vertical_spacer);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);

  HBoxLayout_pushButton_run->addItem(horizontal_spacer_pushButton_run);
  HBoxLayout_pushButton_run->addWidget(pushButton_run);

  HBoxLayout_pushButton_kill->addItem(horizontal_spacer_pushButton_kill);
  HBoxLayout_pushButton_kill->addWidget(pushButton_kill);

  HBoxLayout_pushButton_result_load->addItem(horizontal_spacer_pushButton_result_load);
  HBoxLayout_pushButton_result_load->addWidget(pushButton_result_load);

  HBoxLayout_pushButton_results->addItem(horizontal_spacer_pushButton_results);
  HBoxLayout_pushButton_results->addWidget(pushButton_results);

  QObject::connect(pushButton_run, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_run_clicked(bool)));
  QObject::connect(pushButton_kill, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_kill_clicked(bool)));
  QObject::connect(pushButton_result_load, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_result_load_clicked(bool)));
  QObject::connect(pushButton_results, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_results_clicked(bool)));

  isInitialized = true;
}

JobsActionsPanel::~JobsActionsPanel()
{}

void JobsActionsPanel::on_pushButton_run_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx run job " + lineEdit_1->text());
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}

void JobsActionsPanel::on_pushButton_kill_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx kill job " + lineEdit_1->text());
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}

void JobsActionsPanel::on_pushButton_result_load_clicked(bool)
{
  if (lineEdit_1->text().isEmpty())
  {
    return;
  }
  const std::string job = lineEdit_1->text().toStdString();
  CalculiXApplicationInterface application;
  if (application.cmd("ccx result load job " + job))
  {
    application.cmd("ccx result convert job " + job);
  }
}

void JobsActionsPanel::on_pushButton_results_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("post open ccx job " + lineEdit_1->text());
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}