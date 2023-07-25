#include "FieldOutputsModifyNodePanel.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"


FieldOutputsModifyNodePanel::FieldOutputsModifyNodePanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  keys = ccx_iface->get_fieldoutput_node_keys();

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
  HBoxLayout_10 = new QHBoxLayout();
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
  label_10 = new QLabel(); 
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_9->setFixedWidth(labelWidth);
  label_10->setFixedWidth(labelWidth);
  label_0->setText("Field Output ID");
  label_1->setText("Name");
  label_2->setText("Nodeset ID");
  label_3->setText("Frequency");
  label_4->setText("FrequencyF");
  label_5->setText("Totals");
  label_6->setText("Global");
  label_7->setText("Output");
  label_8->setText("Output All");
  label_9->setText("Last Iterations");
  label_10->setText("Contact Elements");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  lineEdit_4->setPlaceholderText("Optional");
  comboBox_5 = new QComboBox();
  comboBox_5->addItem("");
  comboBox_5->addItem("YES");
  comboBox_5->addItem("ONLY");
  comboBox_5->addItem("NO");
  comboBox_6 = new QComboBox();
  comboBox_6->addItem("");
  comboBox_6->addItem("YES");
  comboBox_6->addItem("NO");
  comboBox_7 = new QComboBox();
  comboBox_7->addItem("");
  comboBox_7->addItem("2D");
  comboBox_7->addItem("3D");
  comboBox_7->addItem("OFF");
  comboBox_8 = new QComboBox();
  comboBox_8->addItem("");
  comboBox_8->addItem("ON");
  comboBox_8->addItem("OFF");
  comboBox_9 = new QComboBox();
  comboBox_9->addItem("");
  comboBox_9->addItem("ON");
  comboBox_9->addItem("OFF");
  comboBox_10 = new QComboBox();
  comboBox_10->addItem("");
  comboBox_10->addItem("ON");
  comboBox_10->addItem("OFF");
  for (size_t i = 0; i < keys.size(); i++)
  {
    HBoxLayout_keys.push_back(new QHBoxLayout());
    label_keys.push_back(new QLabel());
    label_keys[label_keys.size()-1]->setFixedWidth(labelWidth);
    label_keys[label_keys.size()-1]->setText(QString::fromStdString(keys[i]));
    checkBox_keys.push_back(new QCheckBox());
  }

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
  VBoxLayout->addLayout(HBoxLayout_10);
  for (size_t i = 0; i < HBoxLayout_keys.size(); i++)
  {
    VBoxLayout->addLayout(HBoxLayout_keys[i]);
  }
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(comboBox_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(comboBox_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(comboBox_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(comboBox_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(comboBox_9);
  HBoxLayout_10->addWidget(label_10);
  HBoxLayout_10->addWidget(comboBox_10);
  for (size_t i = 0; i < HBoxLayout_keys.size(); i++)
  {
    HBoxLayout_keys[i]->addWidget(label_keys[i]);
    HBoxLayout_keys[i]->addWidget(checkBox_keys[i]);
  }
    
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

FieldOutputsModifyNodePanel::~FieldOutputsModifyNodePanel()
{}

void FieldOutputsModifyNodePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";
  bool key_on = false;
  bool key_off = false;

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify fieldoutput " +lineEdit_0->text() + " node");

    if (lineEdit_1->text() != "")
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
    if (lineEdit_2->text() != "")
    {
      command.append(" nodeset " + lineEdit_2->text());
    }
    if (lineEdit_3->text() != "")
    {
      command.append(" frequency " + lineEdit_3->text());
    }
    if (lineEdit_4->text() != "")
    {
      command.append(" frequencyf " + lineEdit_4->text());
    }
    if (comboBox_5->currentIndex()==1)
    {
      command.append(" totals_yes");
    }else if (comboBox_5->currentIndex()==2)
    {
      command.append(" totals_only");
    }else if (comboBox_5->currentIndex()==3)
    {
      command.append(" totals_no");
    }
    if (comboBox_6->currentIndex()==1)
    {
      command.append(" global_yes");
    }else if (comboBox_6->currentIndex()==2)
    {
      command.append(" global_no");
    }
    if (comboBox_7->currentIndex()==1)
    {
      command.append(" output_2d");
    }else if (comboBox_7->currentIndex()==2)
    {
      command.append(" output_3d");
    }else if (comboBox_7->currentIndex()==3)
    {
      command.append(" output_off");
    }
    if (comboBox_8->currentIndex()==1)
    {
      command.append(" output_all_on");
    }else if (comboBox_8->currentIndex()==2)
    {
      command.append(" output_all_off");
    }
    if (comboBox_9->currentIndex()==1)
    {
      command.append(" last_iterations_on");
    }else if (comboBox_9->currentIndex()==2)
    {
      command.append(" last_iterations_off");
    }
    if (comboBox_10->currentIndex()==1)
    {
      command.append(" contact_elements_on");
    }else if (comboBox_10->currentIndex()==2)
    {
      command.append(" contact_elements_off");
    }
    commands.push_back(command);

    for (size_t i = 0; i < checkBox_keys.size(); i++)
    {
      if (checkBox_keys[i]->isChecked())
      {
        key_on = true;
      }
      if (!checkBox_keys[i]->isChecked())
      {
        key_off = true;
      }
    }
    
    if (key_on)
    {
      command = "ccx modify fieldoutput " +lineEdit_0->text() + " node key_on ";
      for (size_t i = 0; i < checkBox_keys.size(); i++)
      {
        if (checkBox_keys[i]->isChecked())
        {
          command.append(label_keys[i]->text() + " ");
        }
      }
      commands.push_back(command);
    }
    if (key_off)
    {
      command = "ccx modify fieldoutput " +lineEdit_0->text() + " node key_off ";
      for (size_t i = 0; i < checkBox_keys.size(); i++)
      {
        if (!checkBox_keys[i]->isChecked())
        {
          command.append(label_keys[i]->text() + " ");
        }
      }
      commands.push_back(command);
    }
  }
  
  if (command != "")
  {
    lineEdit_0->setText("");
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    lineEdit_3->setText("");
    lineEdit_4->setText("");
    comboBox_5->setCurrentIndex(0);
    comboBox_6->setCurrentIndex(0);
    comboBox_7->setCurrentIndex(0);
    comboBox_8->setCurrentIndex(0);
    comboBox_9->setCurrentIndex(0);
    comboBox_10->setCurrentIndex(0);
    for (size_t i = 0; i < checkBox_keys.size(); i++)
    {
      checkBox_keys[i]->setChecked(false);
    }
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
