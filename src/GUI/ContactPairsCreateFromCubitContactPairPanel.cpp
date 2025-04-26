#include "ContactPairsCreateFromCubitContactPairPanel.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


ContactPairsCreateFromCubitContactPairPanel::ContactPairsCreateFromCubitContactPairPanel(QWidget *parent) :
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
  radioButton_3 = new QRadioButton();
  radioButton_3->setChecked(true);
  radioButton_4 = new QRadioButton();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth-20);
  label_4->setFixedWidth(labelWidth-20);
  label_1->setText("Surface \nIneraction ID");
  label_2->setText("Type");
  label_3->setText("Adjust");
  label_4->setText("Adjust Nodeset ID");
  lineEdit_1 = new QLineEdit();
  comboBox_2 = new QComboBox();
  comboBox_2->addItem("nodetosurface");
  comboBox_2->addItem("surfacetosurface");
  comboBox_2->addItem("mortar");
  comboBox_2->addItem("linmortar");
  comboBox_2->addItem("pglinmortar");
  comboBox_2->addItem("massless");
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  lineEdit_4->setPlaceholderText("Optional");
  lineEdit_4->setDisabled(true);

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
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(comboBox_2);
  HBoxLayout_3->addWidget(radioButton_3);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(radioButton_4);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(radioButton_3, SIGNAL(toggled(bool)),this,  SLOT(on_radioButton_3_toggled(bool)));
  QObject::connect(radioButton_4, SIGNAL(toggled(bool)),this,  SLOT(on_radioButton_4_toggled(bool)));

  isInitialized = true;
}

ContactPairsCreateFromCubitContactPairPanel::~ContactPairsCreateFromCubitContactPairPanel()
{}

void ContactPairsCreateFromCubitContactPairPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx create contactpair from cubitcontactpair surfaceinteraction " +lineEdit_1->text());
    command.append(" " + comboBox_2->currentText());
    
    if((lineEdit_3->isEnabled()) && (lineEdit_3->text()!=""))
    {
      command.append(" adjust " +lineEdit_3->text());
    }else if ((lineEdit_4->isEnabled()) && (lineEdit_4->text()!=""))
    {
      command.append(" adjust_nodeset " +lineEdit_4->text());
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    comboBox_2->setCurrentIndex(0);
    lineEdit_3->setText("");
    lineEdit_4->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}

void ContactPairsCreateFromCubitContactPairPanel::on_radioButton_3_toggled(bool toggled)
{
  lineEdit_3->setDisabled(false);
  lineEdit_4->setDisabled(true);
}

void ContactPairsCreateFromCubitContactPairPanel::on_radioButton_4_toggled(bool toggled)
{
  lineEdit_3->setDisabled(true);
  lineEdit_4->setDisabled(false);
}
