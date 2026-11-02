#include <algorithm>
#include "CubitMessage.hpp"
#include "FieldOutputsModifyContactPanel.hpp"
#include <QString>
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


FieldOutputsModifyContactPanel::FieldOutputsModifyContactPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  CalculiXStepsInterface ccx_steps_iface;

  if(isInitialized)
  {
    return;
  }
  keys = ccx_steps_iface.get_fieldoutput_contact_keys();

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
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_0->setText("Field Output ID");
  label_1->setText("Name");
  label_2->setText("Frequency");
  label_3->setText("Last Iterations");
  label_4->setText("Contact Elements");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  comboBox_3 = new QComboBox();
  comboBox_3->addItem("");
  comboBox_3->addItem("ON");
  comboBox_3->addItem("OFF");
  comboBox_4 = new QComboBox();
  comboBox_4->addItem("");
  comboBox_4->addItem("ON");
  comboBox_4->addItem("OFF");
  
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
  HBoxLayout_3->addWidget(comboBox_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(comboBox_4);
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

FieldOutputsModifyContactPanel::~FieldOutputsModifyContactPanel()
{}

void FieldOutputsModifyContactPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("comboBox_3", comboBox_3->currentText());
  changed = changed || fieldChanged("comboBox_4", comboBox_4->currentText());
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
        setFieldOutput(requestedId);
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
    PRINT_ERROR("An empty value cannot clear Frequency; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";
  bool key_on = false;
  bool key_off = false;

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify fieldoutput " +lineEdit_0->text() + " contact");

    if (lineEdit_1->text() != "" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
    if (lineEdit_2->text() != "" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" frequency " + lineEdit_2->text());
    }
    if (comboBox_3->currentIndex()==1 && fieldChanged("comboBox_3", comboBox_3->currentText()))
    {
      command.append(" last_iterations_on");
    }else if (comboBox_3->currentIndex()==2 && fieldChanged("comboBox_3", comboBox_3->currentText()))
    {
      command.append(" last_iterations_off");
    }
    if (comboBox_4->currentIndex()==1 && fieldChanged("comboBox_4", comboBox_4->currentText()))
    {
      command.append(" contact_elements_on");
    }else if (comboBox_4->currentIndex()==2 && fieldChanged("comboBox_4", comboBox_4->currentText()))
    {
      command.append(" contact_elements_off");
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
      command = "ccx modify fieldoutput " +lineEdit_0->text() + " contact key_on ";
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
      command = "ccx modify fieldoutput " +lineEdit_0->text() + " contact key_off ";
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


bool FieldOutputsModifyContactPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool FieldOutputsModifyContactPanel::setFieldOutput(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_3->setCurrentIndex(0);
  comboBox_4->setCurrentIndex(0);
  for (auto* key : checkBox_keys)
  {
    key->setChecked(false);
  }
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  CalculiXStepsInterface core;
  FieldOutputsSettings settings;
  std::string error;

  if (!core.get_fieldoutput_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load FieldOutput %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 3)
  {
    PRINT_ERROR("The selected record does not match this FieldOutput panel.\n");
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  lineEdit_2->setText(QString::fromStdString(settings.parameters.at(1)));
  const QString comboBox_3Value = QString::fromStdString(settings.parameters.at(3));
  int comboBox_3Index = comboBox_3->findText(comboBox_3Value, Qt::MatchFixedString);
  if (comboBox_3Index < 0 && !comboBox_3Value.isEmpty())
  {
    comboBox_3->addItem(comboBox_3Value);
    comboBox_3Index = comboBox_3->count() - 1;
  }
  comboBox_3->setCurrentIndex(qMax(0, comboBox_3Index));
  const QString comboBox_4Value = QString::fromStdString(settings.parameters.at(4));
  int comboBox_4Index = comboBox_4->findText(comboBox_4Value, Qt::MatchFixedString);
  if (comboBox_4Index < 0 && !comboBox_4Value.isEmpty())
  {
    comboBox_4->addItem(comboBox_4Value);
    comboBox_4Index = comboBox_4->count() - 1;
  }
  comboBox_4->setCurrentIndex(qMax(0, comboBox_4Index));

  for (std::size_t index = 0; index < keys.size(); ++index)
  {
    checkBox_keys[index]->setChecked(std::find(settings.enabledKeys.begin(), settings.enabledKeys.end(), keys[index]) != settings.enabledKeys.end());
    originalValues.insert("key:" + QString::fromStdString(keys[index]), checkBox_keys[index]->isChecked() ? "YES" : "NO");
  }
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("comboBox_3", comboBox_3->currentText());
  originalValues.insert("comboBox_4", comboBox_4->currentText());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
