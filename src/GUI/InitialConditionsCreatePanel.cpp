#include "InitialConditionsCreatePanel.hpp"
#include "PanelTable.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


InitialConditionsCreatePanel::InitialConditionsCreatePanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  //HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  //label_1 = new QLabel();
  label_2 = new QLabel();
  //label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  //label_1->setText("Name");
  label_2->setText("Type");
  //lineEdit_1 = new QLineEdit();
  comboBox_2 = new QComboBox();
  comboBox_2->addItem("Displacement");
  comboBox_2->addItem("Temperature");
  //lineEdit_1->setPlaceholderText("Optional");
  
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  //VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  //HBoxLayout_1->addWidget(label_1);
  //HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(comboBox_2);
    
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

InitialConditionsCreatePanel::~InitialConditionsCreatePanel()
{}

void InitialConditionsCreatePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  //if ((lineEdit_1->text()!=""))
  //{
    command.append("ccx create initialcondition");
       
    if (comboBox_2->currentIndex()==0)
    {
      command.append(" displacement");
    }else if (comboBox_2->currentIndex()==1)
    {
      command.append(" temperature");
    }
  //}
  
  if (command != "")
  {
    commands.push_back(command);
    //lineEdit_1->setText("");
    comboBox_2->setCurrentIndex(0);
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[i].toStdString().c_str());
  }
}
