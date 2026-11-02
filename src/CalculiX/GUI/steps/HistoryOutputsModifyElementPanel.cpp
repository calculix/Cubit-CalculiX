#include <algorithm>
#include "CubitMessage.hpp"
#include "HistoryOutputsModifyElementPanel.hpp"
#include <QString>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


HistoryOutputsModifyElementPanel::HistoryOutputsModifyElementPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  CalculiXStepsInterface ccx_steps_iface;

  if(isInitialized)
  {
    return;
  }
  keys = ccx_steps_iface.get_historyoutput_element_keys();

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
  label_2->setText("Block ID");
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

HistoryOutputsModifyElementPanel::~HistoryOutputsModifyElementPanel()
{}

void HistoryOutputsModifyElementPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
  changed = changed || fieldChanged("lineEdit_4", lineEdit_4->text());
  changed = changed || fieldChanged("comboBox_5", comboBox_5->currentText());
  changed = changed || fieldChanged("comboBox_6", comboBox_6->currentText());
  for (std::size_t index = 0; index < keys.size(); ++index)
  {
    changed = changed || fieldChanged("key:" + QString::fromStdString(keys[index]), checkBox_keys[index]->isChecked() ? "YES" : "NO");
  }
  if (loadedEntityId >= 0 && !changed && lineEdit_0->text().trimmed() == QString::number(loadedEntityId))
  {
    PRINT_INFO("No settings were changed.\n");
    return;
  }

  if (loadedEntityId >= 0)
  {
    if (lineEdit_0->text().trimmed() != QString::number(loadedEntityId))
    {
      bool valid = false;
      const int requestedId = lineEdit_0->text().trimmed().toInt(&valid);

      if (valid)
      {
        setHistoryOutput(requestedId);
      }
      else
      {
        PRINT_ERROR("Select a single entity ID before reloading this panel.\n");
      }

      return;
    }
  }

  if (loadedEntityId >= 0 && lineEdit_1->isEnabled() && !originalValues.value("lineEdit_1").isEmpty() && lineEdit_1->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Name; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_2->isEnabled() && !originalValues.value("lineEdit_2").isEmpty() && lineEdit_2->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Block ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_3->isEnabled() && !originalValues.value("lineEdit_3").isEmpty() && lineEdit_3->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Frequency; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_4->isEnabled() && !originalValues.value("lineEdit_4").isEmpty() && lineEdit_4->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear FrequencyF; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";
  bool key_on = false;
  bool key_off = false;

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify historyoutput " +lineEdit_0->text() + " element");

    if (lineEdit_1->text() != "" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
    if (lineEdit_2->text() != "" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" block " + lineEdit_2->text());
    }
    if (lineEdit_3->text() != "" && fieldChanged("lineEdit_3", lineEdit_3->text()))
    {
      command.append(" frequency " + lineEdit_3->text());
    }
    if (lineEdit_4->text() != "" && fieldChanged("lineEdit_4", lineEdit_4->text()))
    {
      command.append(" frequencyf " + lineEdit_4->text());
    }
    if (comboBox_5->currentIndex()==1 && fieldChanged("comboBox_5", comboBox_5->currentText()))
    {
      command.append(" totals_yes");
    }else if (comboBox_5->currentIndex()==2 && fieldChanged("comboBox_5", comboBox_5->currentText()))
    {
      command.append(" totals_only");
    }else if (comboBox_5->currentIndex()==3 && fieldChanged("comboBox_5", comboBox_5->currentText()))
    {
      command.append(" totals_no");
    }
    if (comboBox_6->currentIndex()==1 && fieldChanged("comboBox_6", comboBox_6->currentText()))
    {
      command.append(" global_yes");
    }else if (comboBox_6->currentIndex()==2 && fieldChanged("comboBox_6", comboBox_6->currentText()))
    {
      command.append(" global_no");
    }
    commands.push_back(command);

    for (size_t i = 0; i < checkBox_keys.size(); i++)
    {
      if (checkBox_keys[i]->isChecked() && fieldChanged("key:" + QString::fromStdString(keys[i]), "YES"))
      {
        key_on = true;
      }
      if (!checkBox_keys[i]->isChecked() && fieldChanged("key:" + QString::fromStdString(keys[i]), "NO"))
      {
        key_off = true;
      }
    }
    
    if (key_on)
    {
      command = "ccx modify historyoutput " +lineEdit_0->text() + " element key_on ";
      for (size_t i = 0; i < checkBox_keys.size(); i++)
      {
        if (checkBox_keys[i]->isChecked() && fieldChanged("key:" + QString::fromStdString(keys[i]), "YES"))
        {
          command.append(label_keys[i]->text() + " ");
        }
      }
      commands.push_back(command);
    }
    if (key_off)
    {
      command = "ccx modify historyoutput " +lineEdit_0->text() + " element key_off ";
      for (size_t i = 0; i < checkBox_keys.size(); i++)
      {
        if (!checkBox_keys[i]->isChecked() && fieldChanged("key:" + QString::fromStdString(keys[i]), "NO"))
        {
          command.append(label_keys[i]->text() + " ");
        }
      }
      commands.push_back(command);
    }
  }
  
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}


bool HistoryOutputsModifyElementPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool HistoryOutputsModifyElementPanel::setHistoryOutput(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_5->setCurrentIndex(0);
  comboBox_6->setCurrentIndex(0);
  for (auto* key : checkBox_keys)
  {
    key->setChecked(false);
  }
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_3->clear();
  lineEdit_4->clear();
  CalculiXStepsInterface core;
  HistoryOutputsSettings settings;
  std::string error;

  if (!core.get_historyoutput_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load HistoryOutput %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 2)
  {
    PRINT_ERROR("The selected record does not match this HistoryOutput panel.\n");
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  lineEdit_2->setText(QString::fromStdString(settings.parameters.at(1)));
  lineEdit_3->setText(QString::fromStdString(settings.parameters.at(2)));
  lineEdit_4->setText(QString::fromStdString(settings.parameters.at(3)));
  const QString comboBox_5Value = QString::fromStdString(settings.parameters.at(4));
  int comboBox_5Index = comboBox_5->findText(comboBox_5Value, Qt::MatchFixedString);
  if (comboBox_5Index < 0 && !comboBox_5Value.isEmpty())
  {
    comboBox_5->addItem(comboBox_5Value);
    comboBox_5Index = comboBox_5->count() - 1;
  }
  comboBox_5->setCurrentIndex(qMax(0, comboBox_5Index));
  const QString comboBox_6Value = QString::fromStdString(settings.parameters.at(5));
  int comboBox_6Index = comboBox_6->findText(comboBox_6Value, Qt::MatchFixedString);
  if (comboBox_6Index < 0 && !comboBox_6Value.isEmpty())
  {
    comboBox_6->addItem(comboBox_6Value);
    comboBox_6Index = comboBox_6->count() - 1;
  }
  comboBox_6->setCurrentIndex(qMax(0, comboBox_6Index));

  for (std::size_t index = 0; index < keys.size(); ++index)
  {
    checkBox_keys[index]->setChecked(std::find(settings.enabledKeys.begin(), settings.enabledKeys.end(), keys[index]) != settings.enabledKeys.end());
    originalValues.insert("key:" + QString::fromStdString(keys[index]), checkBox_keys[index]->isChecked() ? "YES" : "NO");
  }
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("lineEdit_3", lineEdit_3->text());
  originalValues.insert("lineEdit_4", lineEdit_4->text());
  originalValues.insert("comboBox_5", comboBox_5->currentText());
  originalValues.insert("comboBox_6", comboBox_6->currentText());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
