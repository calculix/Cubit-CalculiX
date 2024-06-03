#include "BCsDisplacementsModifyPanel.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


BCsDisplacementsModifyPanel::BCsDisplacementsModifyPanel(QWidget *parent) :
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
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout();
  HBoxLayout_6 = new QHBoxLayout();
  HBoxLayout_7 = new QHBoxLayout();
  HBoxLayout_8 = new QHBoxLayout();
  HBoxLayout_9 = new QHBoxLayout();
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_9 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_0->setText("Displacement ID");
  label_1->setText("OP");
  label_2->setText("Amplitude ID");
  label_3->setText("Time Delay");
  label_4->setText("Fixed DOF 1");
  label_5->setText("Fixed DOF 2");
  label_6->setText("Fixed DOF 3");
  label_7->setText("Fixed DOF 4");
  label_8->setText("Fixed DOF 5");
  label_9->setText("Fixed DOF 6");
  lineEdit_0 = new QLineEdit();
  comboBox_1 = new QComboBox();
  comboBox_1->addItem("");
  comboBox_1->addItem("mod");
  comboBox_1->addItem("new");
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  checkBox_4 = new QCheckBox();
  checkBox_5 = new QCheckBox();
  checkBox_6 = new QCheckBox();
  checkBox_7 = new QCheckBox();
  checkBox_8 = new QCheckBox();
  checkBox_9 = new QCheckBox();
  
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
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addLayout(HBoxLayout_5);
  VBoxLayout->addLayout(HBoxLayout_6);
  VBoxLayout->addLayout(HBoxLayout_7);
  VBoxLayout->addLayout(HBoxLayout_8);
  VBoxLayout->addLayout(HBoxLayout_9);
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
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(checkBox_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(checkBox_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(checkBox_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(checkBox_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(checkBox_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(checkBox_9);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  
  isInitialized = true;
}

BCsDisplacementsModifyPanel::~BCsDisplacementsModifyPanel()
{}

void BCsDisplacementsModifyPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify displacement " +lineEdit_0->text());
    
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
    if (checkBox_4->isChecked())
    {
      command.append(" fixed_1 ");
    }else{
      command.append(" unfixed_1 ");
    }
    if (checkBox_5->isChecked())
    {
      command.append(" fixed_2 ");
    }else{
      command.append(" unfixed_2 ");
    }
    if (checkBox_6->isChecked())
    {
      command.append(" fixed_3 ");
    }else{
      command.append(" unfixed_3 ");
    }
    if (checkBox_7->isChecked())
    {
      command.append(" fixed_4 ");
    }else{
      command.append(" unfixed_4 ");
    }
    if (checkBox_8->isChecked())
    {
      command.append(" fixed_5 ");
    }else{
      command.append(" unfixed_5 ");
    }
    if (checkBox_9->isChecked())
    {
      command.append(" fixed_6 ");
    }else{
      command.append(" unfixed_6 ");
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_0->setText("");
    comboBox_1->setCurrentIndex(0);
    lineEdit_2->setText("");
    lineEdit_3->setText("");
    checkBox_4->setChecked(false);
    checkBox_5->setChecked(false);
    checkBox_6->setChecked(false);
    checkBox_7->setChecked(false);
    checkBox_8->setChecked(false);
    checkBox_9->setChecked(false);
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[int(i)].toStdString().c_str());
  }
}