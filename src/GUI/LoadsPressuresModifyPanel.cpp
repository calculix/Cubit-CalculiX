#include "LoadsPressuresModifyPanel.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


LoadsPressuresModifyPanel::LoadsPressuresModifyPanel(QWidget *parent) :
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
  HBoxLayout_0 = new QHBoxLayout();
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_0->setText("Pressure ID");
  label_1->setText("OP");
  label_2->setText("Amplitude ID");
  label_3->setText("Time Delay");
  lineEdit_0 = new QLineEdit();
  comboBox_1 = new QComboBox();
  comboBox_1->addItem("");
  comboBox_1->addItem("mod");
  comboBox_1->addItem("new");
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(comboBox_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  
  isInitialized = true;
}

LoadsPressuresModifyPanel::~LoadsPressuresModifyPanel()
{}

void LoadsPressuresModifyPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify pressure " +lineEdit_0->text());
    
    if (comboBox_1->currentIndex()!=0)
    {
      command.append(" op " + comboBox_1->currentText());
    }
    if (lineEdit_2->text()!="")
    {
      command.append(" amplitude " +lineEdit_2->text());
    }
    if (lineEdit_3->text()!="")
    {
      command.append(" timedelay " +lineEdit_3->text());
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_0->setText("");
    comboBox_1->setCurrentIndex(0);
    lineEdit_2->setText("");
    lineEdit_3->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[i].toStdString().c_str());
  }
}