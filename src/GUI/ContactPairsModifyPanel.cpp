#include "ContactPairsModifyPanel.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


ContactPairsModifyPanel::ContactPairsModifyPanel(QWidget *parent) :
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
  radioButton_5 = new QRadioButton();
  radioButton_5->setChecked(true);
  radioButton_6 = new QRadioButton();
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth-20);
  label_6->setFixedWidth(labelWidth-20);
  label_0->setText("Contact Pair ID");
  label_1->setText("Surface \nIneraction ID");
  label_2->setText("Type");
  label_3->setText("Master");
  label_4->setText("Slave");
  label_5->setText("Adjust");
  label_6->setText("Adjust Nodeset ID");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  comboBox_2 = new QComboBox();
  comboBox_2->addItem("");
  comboBox_2->addItem("nodetosurface");
  comboBox_2->addItem("surfacetosurface");
  comboBox_2->addItem("mortar");
  comboBox_2->addItem("linmortar");
  comboBox_2->addItem("pglinmortar");
  comboBox_2->addItem("massless");
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional - Sideset ID");
  lineEdit_4->setPlaceholderText("Optional - Sideset ID");
  lineEdit_5->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("Optional");
  lineEdit_6->setDisabled(true);

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
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(comboBox_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(radioButton_5);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(radioButton_6);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(radioButton_5, SIGNAL(toggled(bool)),this,  SLOT(on_radioButton_5_toggled(bool)));
  QObject::connect(radioButton_6, SIGNAL(toggled(bool)),this,  SLOT(on_radioButton_6_toggled(bool)));

  isInitialized = true;
}

ContactPairsModifyPanel::~ContactPairsModifyPanel()
{}

void ContactPairsModifyPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify contactpair " +lineEdit_0->text());
    
    if (lineEdit_1->text()!="")
    {
      command.append(" surfaceinteraction " +lineEdit_1->text());
    }
    if (comboBox_2->currentIndex()!=0)
    {
      command.append(" " + comboBox_2->currentText());
    }
    if (lineEdit_3->text()!="")
    {
      command.append(" master " +lineEdit_3->text());
    }
    if (lineEdit_4->text()!="")
    {
      command.append(" slave " +lineEdit_4->text());
    }
    if((lineEdit_5->isEnabled()) && (lineEdit_5->text()!=""))
    {
      command.append(" adjust " +lineEdit_5->text());
    }else if ((lineEdit_6->isEnabled()) && (lineEdit_6->text()!=""))
    {
      command.append(" adjust_nodeset " +lineEdit_6->text());
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_0->setText("");
    lineEdit_1->setText("");
    comboBox_2->setCurrentIndex(0);
    lineEdit_3->setText("");
    lineEdit_4->setText("");
    lineEdit_5->setText("");
    lineEdit_6->setText("");
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[int(i)].toStdString().c_str());
  }
}

void ContactPairsModifyPanel::on_radioButton_5_toggled(bool toggled)
{
  lineEdit_5->setDisabled(false);
  lineEdit_6->setDisabled(true);
}

void ContactPairsModifyPanel::on_radioButton_6_toggled(bool toggled)
{
  lineEdit_5->setDisabled(true);
  lineEdit_6->setDisabled(false);
}
