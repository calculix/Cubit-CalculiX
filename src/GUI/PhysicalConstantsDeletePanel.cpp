#include "PhysicalConstantsDeletePanel.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


PhysicalConstantsDeletePanel::PhysicalConstantsDeletePanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  label_1 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_1->setText("Absolute Zero");
  checkBox_1 = new QCheckBox();
  checkBox_1->setChecked(false);
  label_2 = new QLabel();
  label_2->setFixedWidth(labelWidth);
  label_2->setText("Stefan Boltzmann");
  checkBox_2 = new QCheckBox();
  checkBox_2->setChecked(false);
  label_3 = new QLabel();
  label_3->setFixedWidth(labelWidth);
  label_3->setText("Newton Gravity");
  checkBox_3 = new QCheckBox();
  checkBox_3->setChecked(false);
  
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(checkBox_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(checkBox_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(checkBox_3);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

PhysicalConstantsDeletePanel::~PhysicalConstantsDeletePanel()
{}

void PhysicalConstantsDeletePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((checkBox_1->isChecked())||(checkBox_2->isChecked())||(checkBox_3->isChecked()))
  {
    command.append("ccx delete physicalconstants");
    if (checkBox_1->isChecked())
    {
      command.append(" absolute_zero");
    }
    if (checkBox_2->isChecked())
    {
      command.append(" stefan_boltzmann");
    }
    if (checkBox_3->isChecked())
    {
      command.append(" newton_gravity");
    }
  }

  if (command != "")
  {
    commands.push_back(command);
    checkBox_1->setChecked(false);
    checkBox_2->setChecked(false); 
    checkBox_3->setChecked(false); 
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}