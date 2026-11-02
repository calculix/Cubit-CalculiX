#include "CubitMessage.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "StepsModifyParameterPanel.hpp"
#include <QString>
#include "GUI/common/PanelTable.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


StepsModifyParameterPanel::StepsModifyParameterPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
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
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_9 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_9->setFixedWidth(labelWidth);
  label_1->setText("Step ID");
  label_2->setText("Name");
  label_3->setText("Perturbation");
  label_4->setText("Nonlinear Geometry");
  label_5->setText("Increment");
  label_6->setText("Increment Fluid");
  label_7->setText("Thermal Network");
  label_8->setText("Amplitude Type");
  label_9->setText("Shocksmoothing");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  comboBox_3 = new QComboBox();
  comboBox_3->addItem("");
  comboBox_3->addItem("YES");
  comboBox_3->addItem("NO");
  comboBox_4 = new QComboBox();
  comboBox_4->addItem("");
  comboBox_4->addItem("YES");
  comboBox_4->addItem("NO");
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  comboBox_7 = new QComboBox();
  comboBox_7->addItem("");
  comboBox_7->addItem("YES");
  comboBox_7->addItem("NO");
  comboBox_8 = new QComboBox();
  comboBox_8->addItem("");
  comboBox_8->addItem("RAMP");
  comboBox_8->addItem("STEP");
  lineEdit_9 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("Optional");
  lineEdit_9->setPlaceholderText("Optional");

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
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(comboBox_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(comboBox_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(lineEdit_9);
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

StepsModifyParameterPanel::~StepsModifyParameterPanel()
{}

void StepsModifyParameterPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("comboBox_3", comboBox_3->currentText());
  changed = changed || fieldChanged("comboBox_4", comboBox_4->currentText());
  changed = changed || fieldChanged("lineEdit_5", lineEdit_5->text());
  changed = changed || fieldChanged("lineEdit_6", lineEdit_6->text());
  changed = changed || fieldChanged("comboBox_7", comboBox_7->currentText());
  changed = changed || fieldChanged("comboBox_8", comboBox_8->currentText());
  changed = changed || fieldChanged("lineEdit_9", lineEdit_9->text());
  if (loadedEntityId >= 0 && !changed && lineEdit_1->text().trimmed() == QString::number(loadedEntityId))
  {
    PRINT_INFO("No settings were changed.\n");
    return;
  }

  if (loadedEntityId >= 0)
  {
    if (lineEdit_1->text().trimmed() != QString::number(loadedEntityId))
    {
      bool valid = false;
      const int requestedId = lineEdit_1->text().trimmed().toInt(&valid);

      if (valid)
      {
        setStep(requestedId);
      }
      else
      {
        PRINT_ERROR("Select a single entity ID before reloading this panel.\n");
      }

      return;
    }
  }

  if (loadedEntityId >= 0 && lineEdit_2->isEnabled() && !originalValues.value("lineEdit_2").isEmpty() && lineEdit_2->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Name; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_5->isEnabled() && !originalValues.value("lineEdit_5").isEmpty() && lineEdit_5->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Increment; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_6->isEnabled() && !originalValues.value("lineEdit_6").isEmpty() && lineEdit_6->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Increment Fluid; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_9->isEnabled() && !originalValues.value("lineEdit_9").isEmpty() && lineEdit_9->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Shocksmoothing; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx modify step " + lineEdit_1->text() + " parameter");
    if (lineEdit_2->text()!="" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" name \"" + lineEdit_2->text() + "\"");
    }
    if (comboBox_3->currentIndex()==1 && fieldChanged("comboBox_3", comboBox_3->currentText()))
    {
      command.append(" perturbation_yes");
    }else if (comboBox_3->currentIndex()==2 && fieldChanged("comboBox_3", comboBox_3->currentText()))
    {
      command.append(" perturbation_no");
    }
    if (comboBox_4->currentIndex()==1 && fieldChanged("comboBox_4", comboBox_4->currentText()))
    {
      command.append(" nlgeom_yes");
    }else if (comboBox_4->currentIndex()==2 && fieldChanged("comboBox_4", comboBox_4->currentText()))
    {
      command.append(" nlgeom_no");
    }
    if (lineEdit_5->text()!="" && fieldChanged("lineEdit_5", lineEdit_5->text()))
    {
      command.append(" inc " + lineEdit_5->text());
    }
    if (lineEdit_6->text()!="" && fieldChanged("lineEdit_6", lineEdit_6->text()))
    {
      command.append(" incf " + lineEdit_6->text());
    }
    if (comboBox_7->currentIndex()==1 && fieldChanged("comboBox_7", comboBox_7->currentText()))
    {
      command.append(" thermalnetwork_yes");
    }else if (comboBox_7->currentIndex()==2 && fieldChanged("comboBox_7", comboBox_7->currentText()))
    {
      command.append(" thermalnetwork_no");
    }
    if (comboBox_8->currentIndex()==1 && fieldChanged("comboBox_8", comboBox_8->currentText()))
    {
      command.append(" amplitude_ramp");
    }else if (comboBox_8->currentIndex()==2 && fieldChanged("comboBox_8", comboBox_8->currentText()))
    {
      command.append(" amplitude_step");
    }
    if (lineEdit_9->text()!="" && fieldChanged("lineEdit_9", lineEdit_9->text()))
    {
      command.append(" shocksmoothing " + lineEdit_9->text());
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


bool StepsModifyParameterPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool StepsModifyParameterPanel::setStep(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_3->setCurrentIndex(0);
  comboBox_4->setCurrentIndex(0);
  comboBox_7->setCurrentIndex(0);
  comboBox_8->setCurrentIndex(0);
  pushButton_apply->setEnabled(false);
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_5->clear();
  lineEdit_6->clear();
  lineEdit_9->clear();
  CalculiXStepsInterface core;
  StepsSettings settings;
  std::string error;

  if (!core.get_step_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Step %d: %s\n", id, error.c_str());
    return false;
  }
  lineEdit_1->setText(QString::number(settings.id));
  lineEdit_2->setText(QString::fromStdString(settings.name));
  const QString comboBox_3Value = QString::fromStdString(settings.parameters.at(1));
  int comboBox_3Index = comboBox_3->findText(comboBox_3Value, Qt::MatchFixedString);
  if (comboBox_3Index < 0 && !comboBox_3Value.isEmpty())
  {
    comboBox_3->addItem(comboBox_3Value);
    comboBox_3Index = comboBox_3->count() - 1;
  }
  comboBox_3->setCurrentIndex(qMax(0, comboBox_3Index));
  const QString comboBox_4Value = QString::fromStdString(settings.parameters.at(2));
  int comboBox_4Index = comboBox_4->findText(comboBox_4Value, Qt::MatchFixedString);
  if (comboBox_4Index < 0 && !comboBox_4Value.isEmpty())
  {
    comboBox_4->addItem(comboBox_4Value);
    comboBox_4Index = comboBox_4->count() - 1;
  }
  comboBox_4->setCurrentIndex(qMax(0, comboBox_4Index));
  lineEdit_5->setText(QString::fromStdString(settings.parameters.at(3)));
  lineEdit_6->setText(QString::fromStdString(settings.parameters.at(4)));
  const QString comboBox_7Value = QString::fromStdString(settings.parameters.at(5));
  int comboBox_7Index = comboBox_7->findText(comboBox_7Value, Qt::MatchFixedString);
  if (comboBox_7Index < 0 && !comboBox_7Value.isEmpty())
  {
    comboBox_7->addItem(comboBox_7Value);
    comboBox_7Index = comboBox_7->count() - 1;
  }
  comboBox_7->setCurrentIndex(qMax(0, comboBox_7Index));
  const QString comboBox_8Value = QString::fromStdString(settings.parameters.at(6)).section('=', -1);
  int comboBox_8Index = comboBox_8->findText(comboBox_8Value, Qt::MatchFixedString);
  if (comboBox_8Index < 0 && !comboBox_8Value.isEmpty())
  {
    comboBox_8->addItem(comboBox_8Value);
    comboBox_8Index = comboBox_8->count() - 1;
  }
  comboBox_8->setCurrentIndex(qMax(0, comboBox_8Index));
  lineEdit_9->setText(QString::fromStdString(settings.parameters.at(7)));
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("comboBox_3", comboBox_3->currentText());
  originalValues.insert("comboBox_4", comboBox_4->currentText());
  originalValues.insert("lineEdit_5", lineEdit_5->text());
  originalValues.insert("lineEdit_6", lineEdit_6->text());
  originalValues.insert("comboBox_7", comboBox_7->currentText());
  originalValues.insert("comboBox_8", comboBox_8->currentText());
  originalValues.insert("lineEdit_9", lineEdit_9->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
