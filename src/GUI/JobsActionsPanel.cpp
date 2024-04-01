#include "JobsActionsPanel.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


JobsActionsPanel::JobsActionsPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
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

  pushButton_result_cgx = new QPushButton();
  pushButton_result_cgx->setText("Result CGX");
  pushButton_result_cgx->setFixedWidth(buttonWidth);
  HBoxLayout_pushButton_result_cgx = new QHBoxLayout();
  horizontal_spacer_pushButton_result_cgx = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  pushButton_result_paraview = new QPushButton();
  pushButton_result_paraview->setText("Result Paraview");
  pushButton_result_paraview->setFixedWidth(buttonWidth);
  HBoxLayout_pushButton_result_paraview = new QHBoxLayout();
  horizontal_spacer_pushButton_result_paraview = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  
  VBoxLayout->addLayout(HBoxLayout_pushButton_run);
  VBoxLayout->addLayout(HBoxLayout_pushButton_kill);
  VBoxLayout->addLayout(HBoxLayout_pushButton_result_cgx);
  VBoxLayout->addLayout(HBoxLayout_pushButton_result_paraview);
  VBoxLayout->addItem(vertical_spacer);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);

  HBoxLayout_pushButton_run->addItem(horizontal_spacer_pushButton_run);
  HBoxLayout_pushButton_run->addWidget(pushButton_run);

  HBoxLayout_pushButton_kill->addItem(horizontal_spacer_pushButton_kill);
  HBoxLayout_pushButton_kill->addWidget(pushButton_kill);

  HBoxLayout_pushButton_result_cgx->addItem(horizontal_spacer_pushButton_result_cgx);
  HBoxLayout_pushButton_result_cgx->addWidget(pushButton_result_cgx);

  HBoxLayout_pushButton_result_paraview->addItem(horizontal_spacer_pushButton_result_paraview);
  HBoxLayout_pushButton_result_paraview->addWidget(pushButton_result_paraview);

  QObject::connect(pushButton_run, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_run_clicked(bool)));
  QObject::connect(pushButton_kill, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_kill_clicked(bool)));
  QObject::connect(pushButton_result_cgx, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_result_cgx_clicked(bool)));
  QObject::connect(pushButton_result_paraview, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_result_paraview_clicked(bool)));

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
    CubitInterface::cmd(commands[i].toStdString().c_str());
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
    CubitInterface::cmd(commands[i].toStdString().c_str());
  }
}

void JobsActionsPanel::on_pushButton_result_cgx_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx result cgx job " + lineEdit_1->text());
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[i].toStdString().c_str());
  }
}

void JobsActionsPanel::on_pushButton_result_paraview_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx result paraview job " + lineEdit_1->text());
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[i].toStdString().c_str());
  }
}