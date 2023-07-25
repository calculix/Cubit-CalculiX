#include "StepsModifyHeatTransferPanel.hpp"
#include "PanelTable.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "ScriptTranslator.hpp"


StepsModifyHeatTransferPanel::StepsModifyHeatTransferPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
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
  HBoxLayout_10 = new QHBoxLayout();
  HBoxLayout_11 = new QHBoxLayout();
  HBoxLayout_12 = new QHBoxLayout();
  HBoxLayout_13 = new QHBoxLayout();
  HBoxLayout_14 = new QHBoxLayout();
  HBoxLayout_15 = new QHBoxLayout();
  HBoxLayout_16 = new QHBoxLayout();
  HBoxLayout_17 = new QHBoxLayout();
  HBoxLayout_18 = new QHBoxLayout();
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
  label_11 = new QLabel();
  label_12 = new QLabel();
  label_13 = new QLabel();
  label_14 = new QLabel();
  label_15 = new QLabel();
  label_16 = new QLabel();
  label_17 = new QLabel();
  label_18 = new QLabel();
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
  label_11->setFixedWidth(labelWidth);
  label_12->setFixedWidth(labelWidth);
  label_13->setFixedWidth(labelWidth);
  label_14->setFixedWidth(labelWidth);
  label_15->setFixedWidth(labelWidth);
  label_16->setFixedWidth(labelWidth);
  label_17->setFixedWidth(labelWidth);
  label_18->setFixedWidth(labelWidth);
  label_1->setText("Step ID");
  label_2->setText("Name");
  label_3->setText("Solver");
  label_4->setText("Direct");
  label_5->setText("Steady State");
  label_6->setText("Frequency");
  label_7->setText("Modal Dynamic");
  label_8->setText("Storage");
  label_9->setText("DELTMX");
  label_10->setText("Time Reset");
  label_11->setText("Total Time\nat Start");
  label_12->setText("Initial \nTime Increment");
  label_13->setText("Time Period\nof Step");
  label_14->setText("Minimum \nTime Increment");
  label_15->setText("Maximum \nTime Increment");
  label_16->setText("Eigenfrequencies");
  label_17->setText("lower");
  label_18->setText("upper");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  comboBox_3 = new QComboBox();
  comboBox_3->addItem("");
  comboBox_3->addItem("pastix");
  comboBox_3->addItem("pardiso");
  comboBox_3->addItem("spooles");
  comboBox_4 = new QComboBox();
  comboBox_4->addItem("");
  comboBox_4->addItem("YES");
  comboBox_4->addItem("NO");
  comboBox_5 = new QComboBox();
  comboBox_5->addItem("");
  comboBox_5->addItem("YES");
  comboBox_5->addItem("NO");
  comboBox_6 = new QComboBox();
  comboBox_6->addItem("");
  comboBox_6->addItem("YES");
  comboBox_6->addItem("NO");
  comboBox_7 = new QComboBox();
  comboBox_7->addItem("");
  comboBox_7->addItem("YES");
  comboBox_7->addItem("NO");
  comboBox_8 = new QComboBox();
  comboBox_8->addItem("");
  comboBox_8->addItem("YES");
  comboBox_8->addItem("NO");
  lineEdit_9 = new QLineEdit();
  comboBox_10 = new QComboBox();
  comboBox_10->addItem("");
  comboBox_10->addItem("YES");
  comboBox_10->addItem("NO");
  lineEdit_11 = new QLineEdit();
  lineEdit_12 = new QLineEdit();
  lineEdit_13 = new QLineEdit();
  lineEdit_14 = new QLineEdit();
  lineEdit_15 = new QLineEdit();
  lineEdit_16 = new QLineEdit();
  lineEdit_17 = new QLineEdit();
  lineEdit_18 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  //lineEdit_7->setPlaceholderText("Optional");
  //lineEdit_8->setPlaceholderText("Optional");
  lineEdit_9->setPlaceholderText("Optional");
  //lineEdit_10->setPlaceholderText("Optional");
  lineEdit_11->setPlaceholderText("Optional");
  lineEdit_12->setPlaceholderText("Optional");
  lineEdit_13->setPlaceholderText("Optional");
  lineEdit_14->setPlaceholderText("Optional");
  lineEdit_15->setPlaceholderText("Optional");
  lineEdit_16->setPlaceholderText("Optional");
  lineEdit_17->setPlaceholderText("Optional");
  lineEdit_18->setPlaceholderText("Optional");

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
  VBoxLayout->addLayout(HBoxLayout_10);
  VBoxLayout->addLayout(HBoxLayout_11);
  VBoxLayout->addLayout(HBoxLayout_12);
  VBoxLayout->addLayout(HBoxLayout_13);
  VBoxLayout->addLayout(HBoxLayout_14);
  VBoxLayout->addLayout(HBoxLayout_15);
  VBoxLayout->addLayout(HBoxLayout_16);
  VBoxLayout->addLayout(HBoxLayout_17);
  VBoxLayout->addLayout(HBoxLayout_18);
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
  HBoxLayout_5->addWidget(comboBox_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(comboBox_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(comboBox_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(comboBox_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(lineEdit_9);
  HBoxLayout_10->addWidget(label_10);
  HBoxLayout_10->addWidget(comboBox_10);
  HBoxLayout_11->addWidget(label_11);
  HBoxLayout_11->addWidget(lineEdit_11);
  HBoxLayout_12->addWidget(label_12);
  HBoxLayout_12->addWidget(lineEdit_12);
  HBoxLayout_13->addWidget(label_13);
  HBoxLayout_13->addWidget(lineEdit_13);
  HBoxLayout_14->addWidget(label_14);
  HBoxLayout_14->addWidget(lineEdit_14);
  HBoxLayout_15->addWidget(label_15);
  HBoxLayout_15->addWidget(lineEdit_15);
  HBoxLayout_16->addWidget(label_16);
  HBoxLayout_16->addWidget(lineEdit_16);
  HBoxLayout_17->addWidget(label_17);
  HBoxLayout_17->addWidget(lineEdit_17);
  HBoxLayout_18->addWidget(label_18);
  HBoxLayout_18->addWidget(lineEdit_18);
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

StepsModifyHeatTransferPanel::~StepsModifyHeatTransferPanel()
{}

void StepsModifyHeatTransferPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx modify step " + lineEdit_1->text() + " heattransfer");
    if (lineEdit_2->text()!="")
    {
      command.append(" name \"" + lineEdit_2->text() + "\"");
    }
    if (comboBox_3->currentIndex()==1)
    {
      command.append(" solver pastix");
    }else if (comboBox_3->currentIndex()==2)
    {
      command.append(" solver pardiso");
    }else if (comboBox_3->currentIndex()==3)
    {
      command.append(" solver spooles");
    }
    if (comboBox_4->currentIndex()==1)
    {
      command.append(" direct_yes");
    }else if (comboBox_4->currentIndex()==2)
    {
      command.append(" direct_no");
    }
    if (comboBox_5->currentIndex()==1)
    {
      command.append(" steadystate_yes");
    }else if (comboBox_5->currentIndex()==2)
    {
      command.append(" steadystate_no");
    }
    if (comboBox_6->currentIndex()==1)
    {
      command.append(" frequency_yes");
    }else if (comboBox_6->currentIndex()==2)
    {
      command.append(" frequency_no");
    }
    if (comboBox_7->currentIndex()==1)
    {
      command.append(" modaldynamic_yes");
    }else if (comboBox_7->currentIndex()==2)
    {
      command.append(" modaldynamic_no");
    }
    if (comboBox_8->currentIndex()==1)
    {
      command.append(" storage_yes");
    }else if (comboBox_8->currentIndex()==2)
    {
      command.append(" storage_no");
    }
    if (lineEdit_9->text()!="")
    {
      command.append(" deltmx " + lineEdit_9->text());
    }
    if (comboBox_10->currentIndex()==1)
    {
      command.append(" timereset_yes");
    }else if (comboBox_10->currentIndex()==2)
    {
      command.append(" timereset_no");
    }
    if (lineEdit_11->text()!="")
    {
      command.append(" totaltimeatstart " + lineEdit_11->text());
    }
    if (lineEdit_12->text()!="")
    {
      command.append(" initialtimeincrement " + lineEdit_12->text());
    }
    if (lineEdit_13->text()!="")
    {
      command.append(" timeperiodofstep " + lineEdit_13->text());
    }
    if (lineEdit_14->text()!="")
    {
      command.append(" minimumtimeincrement " + lineEdit_14->text());
    }
    if (lineEdit_15->text()!="")
    {
      command.append(" maximumtimeincrement " + lineEdit_15->text());
    }
    if (lineEdit_16->text()!="")
    {
      command.append(" eigenfrequencies " + lineEdit_16->text());
    }
    if (lineEdit_17->text()!="")
    {
      command.append(" lower " + lineEdit_17->text());
    }
    if (lineEdit_18->text()!="")
    {
      command.append(" upper " + lineEdit_18->text());
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    lineEdit_2->setText("");
    comboBox_3->setCurrentIndex(0);
    comboBox_4->setCurrentIndex(0);
    comboBox_5->setCurrentIndex(0);
    comboBox_6->setCurrentIndex(0);
    comboBox_7->setCurrentIndex(0);
    comboBox_8->setCurrentIndex(0);
    lineEdit_9->setText("");
    comboBox_10->setCurrentIndex(0);
    lineEdit_11->setText("");
    lineEdit_12->setText("");
    lineEdit_13->setText("");
    lineEdit_14->setText("");
    lineEdit_15->setText("");
    lineEdit_16->setText("");
    lineEdit_17->setText("");
    lineEdit_18->setText("");
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
