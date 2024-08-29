#include "StepsModifyParameterPanel.hpp"
#include "PanelTable.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


StepsModifyParameterPanel::StepsModifyParameterPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  int labelWidth = 135;
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
  HBoxLayout_7 = new QHBoxLayout();
  HBoxLayout_8 = new QHBoxLayout();
  HBoxLayout_9 = new QHBoxLayout();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_9 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_9->setFixedWidth(labelWidth);
  label_1->setText("Step ID");
  label_2->setText("Name");
  label_3->setText("Perturbation");
  label_4->setText("Nonlinear Geometry");
  label_5->setText("Increment");
  label_6->setText("Increment Fluid");
  label_7->setText("Thermal Network");
  label_8->setText("Amplitude Type");
  label_9->setText("Shocksmoothing");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  comboBox_3 = new QComboBox();
  comboBox_3->addItem("");
  comboBox_3->addItem("YES");
  comboBox_3->addItem("NO");
  comboBox_4 = new QComboBox();
  comboBox_4->addItem("");
  comboBox_4->addItem("YES");
  comboBox_4->addItem("NO");
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  comboBox_7 = new QComboBox();
  comboBox_7->addItem("");
  comboBox_7->addItem("YES");
  comboBox_7->addItem("NO");
  comboBox_8 = new QComboBox();
  comboBox_8->addItem("");
  comboBox_8->addItem("RAMP");
  comboBox_8->addItem("STEP");
  lineEdit_9 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("Optional");
  lineEdit_9->setPlaceholderText("Optional");

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
  VBoxLayout->addLayout(HBoxLayout_7);
  VBoxLayout->addLayout(HBoxLayout_8);
  VBoxLayout->addLayout(HBoxLayout_9);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(comboBox_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(comboBox_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(comboBox_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(comboBox_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(lineEdit_9);
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

StepsModifyParameterPanel::~StepsModifyParameterPanel()
{}

void StepsModifyParameterPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx modify step " + lineEdit_1->text() + " parameter");
    if (lineEdit_2->text()!="")
    {
      command.append(" name \"" + lineEdit_2->text() + "\"");
    }
    if (comboBox_3->currentIndex()==1)
    {
      command.append(" perturbation_yes");
    }else if (comboBox_3->currentIndex()==2)
    {
      command.append(" perturbation_no");
    }
    if (comboBox_4->currentIndex()==1)
    {
      command.append(" nlgeom_yes");
    }else if (comboBox_4->currentIndex()==2)
    {
      command.append(" nlgeom_no");
    }
    if (lineEdit_5->text()!="")
    {
      command.append(" inc " + lineEdit_5->text());
    }
    if (lineEdit_6->text()!="")
    {
      command.append(" incf " + lineEdit_6->text());
    }
    if (comboBox_7->currentIndex()==1)
    {
      command.append(" thermalnetwork_yes");
    }else if (comboBox_7->currentIndex()==2)
    {
      command.append(" thermalnetwork_no");
    }
    if (comboBox_8->currentIndex()==1)
    {
      command.append(" amplitude_ramp");
    }else if (comboBox_8->currentIndex()==2)
    {
      command.append(" amplitude_step");
    }
    if (lineEdit_9->text()!="")
    {
      command.append(" shocksmoothing " + lineEdit_9->text());
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    comboBox_3->setCurrentIndex(0);
    comboBox_4->setCurrentIndex(0);
    lineEdit_5->setText("");
    lineEdit_6->setText("");
    comboBox_7->setCurrentIndex(0);
    comboBox_8->setCurrentIndex(0);
    lineEdit_9->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}
