#include "ConstraintsCreateEquationFromGroupPanel.hpp"
#include "CalculiXCoreInterface.hpp"
#include "PanelTable.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


ConstraintsCreateEquationFromGroupPanel::ConstraintsCreateEquationFromGroupPanel(QWidget *parent) :
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
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout();
  HBoxLayout_6 = new QHBoxLayout();
  label_1 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_1->setText("Name");
  label_2 = new QLabel();
  label_2->setFixedWidth(labelWidth);
  label_2->setText("Group ID");
  label_3 = new QLabel();
  label_3->setFixedWidth(labelWidth);
  label_3->setText("Tolerance");
  label_4 = new QLabel();
  label_4->setFixedWidth(labelWidth);
  label_4->setText("DOF 1");
  label_5 = new QLabel();
  label_5->setFixedWidth(labelWidth);
  label_5->setText("DOF 2");
  label_6 = new QLabel();
  label_6->setFixedWidth(labelWidth);
  label_6->setText("DOF 3");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  checkBox_4 = new QCheckBox();
  checkBox_5 = new QCheckBox();
  checkBox_6 = new QCheckBox();

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addLayout(HBoxLayout_5);
  VBoxLayout->addLayout(HBoxLayout_6);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
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
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  
  isInitialized = true;
}

ConstraintsCreateEquationFromGroupPanel::~ConstraintsCreateEquationFromGroupPanel()
{}

void ConstraintsCreateEquationFromGroupPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";
  
  if ((lineEdit_1->text()!="") && (lineEdit_2->text()!="") && (lineEdit_3->text()!="") && (checkBox_4->isChecked() || checkBox_5->isChecked() || checkBox_6->isChecked()))
  {
    command.append("ccx create constraint equation name \"" + lineEdit_1->text() + "\"");
    
    command.append(" group_id " + lineEdit_2->text());
    command.append(" tolerance " + lineEdit_3->text());

    if (checkBox_4->isChecked())
    {
      command.append(" dof_1 ");
    }
    if (checkBox_5->isChecked())
    {
      command.append(" dof_2 ");
    }
    if (checkBox_6->isChecked())
    {
      command.append(" dof_3 ");
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("");
    checkBox_4->setChecked(false);
    checkBox_5->setChecked(false);
    checkBox_6->setChecked(false);
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}