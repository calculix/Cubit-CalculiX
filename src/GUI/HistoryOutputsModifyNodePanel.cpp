#include "HistoryOutputsModifyNodePanel.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


HistoryOutputsModifyNodePanel::HistoryOutputsModifyNodePanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  keys = ccx_iface->get_historyoutput_node_keys();

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
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_0->setText("History Output ID");
  label_1->setText("Name");
  label_2->setText("Nodeset ID");
  label_3->setText("Frequency");
  label_4->setText("FrequencyF");
  label_5->setText("Totals");
  label_6->setText("Global");
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

HistoryOutputsModifyNodePanel::~HistoryOutputsModifyNodePanel()
{}

void HistoryOutputsModifyNodePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";
  bool key_on = false;
  bool key_off = false;

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify historyoutput " +lineEdit_0->text() + " node");

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
      command = "ccx modify historyoutput " +lineEdit_0->text() + " node key_on ";
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
      command = "ccx modify historyoutput " +lineEdit_0->text() + " node key_off ";
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
    for (size_t i = 0; i < checkBox_keys.size(); i++)
    {
      checkBox_keys[i]->setChecked(false);
    }
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    CubitInterface::cmd(commands[i].toStdString().c_str());
  }
}
