#include "SectionsCreatePanel.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"


SectionsCreate::SectionsCreate(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;

  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_1->setFixedWidth(50);
  label_2->setFixedWidth(50);
  label_3->setFixedWidth(50);
  label_1->setText("Label 1");
  label_2->setText("Label 2");
  label_3->setText("Label 3");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
/*
  ccx delete section <section id>
ccx create section solid block <block id> material <material id> [orientation <orientation_name>] [thickness <thickness>]
ccx modify section solid <section id> [block <block id>] [material <material id>] [orientation <orientation_name>] [thickness <thickness>]
ccx create section shell block <block id> material <material id> [orientation <orientation_name>] [thickness <thickness>] [offset <offset>]
ccx modify section shell <section id> [block <block id>] [material <material id>] [orientation <orientation_name>] [thickness <thickness>] [offset <offset>]
ccx modify section beam <section id> [beam_type <beam_type>] [block <block id>] [material <material id>] [thickness1 <thickness1>] [thickness2 <thickness2>] [x <x>] [y <y>] [z <z>] [orientation <orientation_name>] [offset1 <offset1>] [offset2 <offset2>]
ccx create section membrane block <block id> material <material id> [orientation <orientation_name>] [thickness <thickness>] [offset <offset>]
ccx modify section membrane <section id> [block <block id>] [material <material id>] [orientation <orientation_name>] [thickness <thickness>] [offset <offset>]
*/
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addItem(vertical_spacer);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);

  isInitialized = true;
}

SectionsCreate::~SectionsCreate()
{}

void SectionsCreate::on_pbApply_clicked()
{
  QStringList commands;

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
