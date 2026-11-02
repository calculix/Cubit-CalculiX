#include <algorithm>
#include "CubitMessage.hpp"
#include "HistoryOutputsModifySectionPanel.hpp"
#include <QString>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


HistoryOutputsModifySectionPanel::HistoryOutputsModifySectionPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  CalculiXStepsInterface ccx_steps_iface;

  if(isInitialized)
  {
    return;
  }
  keys = ccx_steps_iface.get_historyoutput_section_keys();

  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_0 = new QHBoxLayout();
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_0->setText("History Output ID");
  label_1->setText("Name");
  label_2->setText("Sideset ID");
  label_3->setText("FrequencyF");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");

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

HistoryOutputsModifySectionPanel::~HistoryOutputsModifySectionPanel()
{}

void HistoryOutputsModifySectionPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
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

  //ccx modify historyoutput <output id> contact [name <name>] [sideset <sideset id>] [frequencyf <frequencyf>] [{key_on|key_off} [SOF] [SOM] [SOAREA] [FLUX] [DRAG]]
  if (loadedEntityId >= 0 && lineEdit_1->isEnabled() && !originalValues.value("lineEdit_1").isEmpty() && lineEdit_1->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Name; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_2->isEnabled() && !originalValues.value("lineEdit_2").isEmpty() && lineEdit_2->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Sideset ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_3->isEnabled() && !originalValues.value("lineEdit_3").isEmpty() && lineEdit_3->text().trimmed().isEmpty())
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
    command.append("ccx modify historyoutput " +lineEdit_0->text() + " section");

    if (lineEdit_1->text() != "" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
    if (lineEdit_2->text() != "" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" sideset " + lineEdit_2->text());
    }
    if (lineEdit_3->text() != "" && fieldChanged("lineEdit_3", lineEdit_3->text()))
    {
      command.append(" frequencyf " + lineEdit_3->text());
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
      command = "ccx modify historyoutput " +lineEdit_0->text() + " section key_on ";
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
      command = "ccx modify historyoutput " +lineEdit_0->text() + " section key_off ";
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
  
  if (command != "")
  {
 
    for (size_t i = 0; i < checkBox_keys.size(); i++)
    {
    }
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}


bool HistoryOutputsModifySectionPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool HistoryOutputsModifySectionPanel::setHistoryOutput(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  for (auto* key : checkBox_keys)
  {
    key->setChecked(false);
  }
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_3->clear();
  CalculiXStepsInterface core;
  HistoryOutputsSettings settings;
  std::string error;

  if (!core.get_historyoutput_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load HistoryOutput %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 4)
  {
    PRINT_ERROR("The selected record does not match this HistoryOutput panel.\n");
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  lineEdit_2->setText(QString::fromStdString(settings.parameters.at(1)));
  lineEdit_3->setText(QString::fromStdString(settings.parameters.at(2)));

  for (std::size_t index = 0; index < keys.size(); ++index)
  {
    checkBox_keys[index]->setChecked(std::find(settings.enabledKeys.begin(), settings.enabledKeys.end(), keys[index]) != settings.enabledKeys.end());
    originalValues.insert("key:" + QString::fromStdString(keys[index]), checkBox_keys[index]->isChecked() ? "YES" : "NO");
  }
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("lineEdit_3", lineEdit_3->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
