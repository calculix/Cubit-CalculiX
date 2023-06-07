#include "ConstraintsCreateRigidBodyPanel.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"


ConstraintsCreateRigidBodyPanel::ConstraintsCreateRigidBodyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  frame_1 = new QFrame();
  frame_1->setLineWidth(1);
  frame_1->setMidLineWidth(0);
  frame_1->setFrameStyle(QFrame::Box | QFrame::Raised);
  VBoxLayout_1 = new QVBoxLayout(frame_1);
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  radioButton_1 = new QRadioButton();
  radioButton_1->setChecked(true);
  radioButton_2 = new QRadioButton();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_1->setFixedWidth(labelWidth-30);
  label_2->setFixedWidth(labelWidth-30);
  label_3->setFixedWidth(labelWidth);
  label_1->setText("Block ID");
  label_2->setText("Nodeset ID");
  label_3->setText("Vertex ID");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_2->setDisabled(true);
  lineEdit_3 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  //lineEdit_2->setPlaceholderText("Optional");
  //lineEdit_3->setPlaceholderText("Optional");
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addWidget(frame_1);
  VBoxLayout_1->addLayout(HBoxLayout_1);
  VBoxLayout_1->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);
  

  HBoxLayout_1->addWidget(radioButton_1);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(radioButton_2);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(radioButton_1, SIGNAL(toggled(bool)),this,  SLOT(on_radioButton_1_toggled(bool)));
  QObject::connect(radioButton_2, SIGNAL(toggled(bool)),this,  SLOT(on_radioButton_2_toggled(bool)));

  isInitialized = true;
}

ConstraintsCreateRigidBodyPanel::~ConstraintsCreateRigidBodyPanel()
{}

void ConstraintsCreateRigidBodyPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!="") && (lineEdit_1->isEnabled()))
  {
    command.append("ccx create constraint rigid body block " + lineEdit_1->text());
    command.append(" vertex " + lineEdit_3->text());
  }else if ((lineEdit_2->text()!="") && (lineEdit_2->isEnabled()))
  {
    command.append("ccx create constraint rigid body nodeset " + lineEdit_2->text());
    command.append(" vertex " + lineEdit_3->text());
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("");
  }
  
  // We must send the Cubit commands through the Claro framework, so first we need to translate
  // the commands into the python form that Claro will understand.
  ScriptTranslator* cubit_translator = Broker::instance()->get_translator("Cubit");
  if(cubit_translator)
  {
    for(int i = 0; i < commands.size(); i++)
      cubit_translator->decode(commands[i]);

    // Send the translated commands
    Claro::instance()->send_gui_commands(commands);
  }
}

void ConstraintsCreateRigidBodyPanel::on_radioButton_1_toggled(bool toggled)
{
  lineEdit_1->setDisabled(false);
  lineEdit_2->setDisabled(true);
}

void ConstraintsCreateRigidBodyPanel::on_radioButton_2_toggled(bool toggled)
{
  lineEdit_1->setDisabled(true);
  lineEdit_2->setDisabled(false);
}