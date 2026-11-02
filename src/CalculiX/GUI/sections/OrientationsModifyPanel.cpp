#include "CubitMessage.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "OrientationsModifyPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


OrientationsModifyPanel::OrientationsModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
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
  label_0->setText("Orientation");
  label_1->setText("Name");
  label_2->setText("System Type");
  label_3->setText("a coord");
  label_4->setText("b coord");
  label_5->setText("local axis");
  label_6->setText("angle");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  comboBox_2 = new QComboBox();
  comboBox_2->addItem(" ");
  comboBox_2->addItem("Rectangular");
  comboBox_2->addItem("Cylindrical");
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  comboBox_5 = new QComboBox();
  comboBox_5->addItem(" ");
  comboBox_5->addItem("x");
  comboBox_5->addItem("y");
  comboBox_5->addItem("z");
  lineEdit_6 = new QLineEdit();

  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional <x> <y> <z>");
  lineEdit_4->setPlaceholderText("Optional <x> <y> <z>");
  lineEdit_6->setPlaceholderText("Optional");

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
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(comboBox_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

OrientationsModifyPanel::~OrientationsModifyPanel()
{}

void OrientationsModifyPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("comboBox_2", comboBox_2->currentText());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
  changed = changed || fieldChanged("lineEdit_4", lineEdit_4->text());
  changed = changed || fieldChanged("comboBox_5", comboBox_5->currentText());
  changed = changed || fieldChanged("lineEdit_6", lineEdit_6->text());
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
        setOrientation(requestedId);
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
  if (loadedEntityId >= 0 && lineEdit_3->isEnabled() && !originalValues.value("lineEdit_3").isEmpty() && lineEdit_3->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear a coord; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_4->isEnabled() && !originalValues.value("lineEdit_4").isEmpty() && lineEdit_4->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear b coord; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_6->isEnabled() && !originalValues.value("lineEdit_6").isEmpty() && lineEdit_6->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear angle; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify orientation " + lineEdit_0->text());

    if (lineEdit_1->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name " + lineEdit_1->text());
    }  
    
    if (comboBox_2->currentIndex()==1 && fieldChanged("comboBox_2", comboBox_2->currentText()))
    {
      command.append(" system_type_r ");
    }else if (comboBox_2->currentIndex()==2 && fieldChanged("comboBox_2", comboBox_2->currentText()))
    {
      command.append(" system_type_c ");
    }
    
    if (lineEdit_3->text()!="" && fieldChanged("lineEdit_3", lineEdit_3->text()))
    {
      command.append(" a_coord " + lineEdit_3->text());
    }

    if (lineEdit_4->text()!="" && fieldChanged("lineEdit_4", lineEdit_4->text()))
    {
      command.append(" b_coord " + lineEdit_4->text());
    }

    if (comboBox_5->currentIndex()==1 && fieldChanged("comboBox_5", comboBox_5->currentText()))
    {
      command.append(" local_axis_x angle " + lineEdit_6->text());
    }else if (comboBox_5->currentIndex()==2 && fieldChanged("comboBox_5", comboBox_5->currentText()))
    {
      command.append(" local_axis_y angle " + lineEdit_6->text());
    }else if (comboBox_5->currentIndex()==3 && fieldChanged("comboBox_5", comboBox_5->currentText()))
    {
      command.append(" local_axis_z angle " + lineEdit_6->text());
    }
    
  }
  
  if (command != "")
  {
    commands.push_back(command);
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}


bool OrientationsModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool OrientationsModifyPanel::setOrientation(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_2->setCurrentIndex(0);
  comboBox_5->setCurrentIndex(0);
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_3->clear();
  lineEdit_4->clear();
  lineEdit_6->clear();
  CalculiXSectionsInterface core;
  OrientationsSettings settings;
  std::string error;

  if (!core.get_orientation_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Orientation %d: %s\n", id, error.c_str());
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  const QString comboBox_2Value = QString::fromStdString(settings.system);
  int comboBox_2Index = comboBox_2->findText(comboBox_2Value, Qt::MatchFixedString);
  if (comboBox_2Index < 0 && !comboBox_2Value.isEmpty())
  {
    comboBox_2->addItem(comboBox_2Value);
    comboBox_2Index = comboBox_2->count() - 1;
  }
  comboBox_2->setCurrentIndex(qMax(0, comboBox_2Index));
  lineEdit_3->setText(QString::fromStdString(settings.a));
  lineEdit_4->setText(QString::fromStdString(settings.b));
  const QString comboBox_5Value = QString::fromStdString(settings.axis);
  int comboBox_5Index = comboBox_5->findText(comboBox_5Value, Qt::MatchFixedString);
  if (comboBox_5Index < 0 && !comboBox_5Value.isEmpty())
  {
    comboBox_5->addItem(comboBox_5Value);
    comboBox_5Index = comboBox_5->count() - 1;
  }
  comboBox_5->setCurrentIndex(qMax(0, comboBox_5Index));
  lineEdit_6->setText(QString::fromStdString(settings.angle));
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("comboBox_2", comboBox_2->currentText());
  originalValues.insert("lineEdit_3", lineEdit_3->text());
  originalValues.insert("lineEdit_4", lineEdit_4->text());
  originalValues.insert("comboBox_5", comboBox_5->currentText());
  originalValues.insert("lineEdit_6", lineEdit_6->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
