#include "CubitMessage.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "StepsModifyCoupledTemperatureDisplacementPanel.hpp"
#include <QString>
#include "GUI/common/PanelTable.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


StepsModifyCoupledTemperatureDisplacementPanel::StepsModifyCoupledTemperatureDisplacementPanel(QWidget *parent) :
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
  HBoxLayout_10 = new QHBoxLayout();
  HBoxLayout_11 = new QHBoxLayout();
  HBoxLayout_12 = new QHBoxLayout();
  HBoxLayout_13 = new QHBoxLayout();
  HBoxLayout_14 = new QHBoxLayout();
  HBoxLayout_15 = new QHBoxLayout();
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
  label_1->setText("Step ID");
  label_2->setText("Name");
  label_3->setText("Solver");
  label_4->setText("Direct");
  label_5->setText("Alpha");
  label_6->setText("Steady State");
  label_7->setText("DELTMX");
  label_8->setText("Time Reset");
  label_9->setText("Total Time\nat Start");
  label_10->setText("Compressible");
  label_11->setText("Initial \nTime Increment");
  label_12->setText("Time Period\nof Step");
  label_13->setText("Minimum \nTime Increment");
  label_14->setText("Maximum \nTime Increment");
  label_15->setText("Initial \nTime Increment CFD");
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
  lineEdit_5 = new QLineEdit();
  comboBox_6 = new QComboBox();
  comboBox_6->addItem("");
  comboBox_6->addItem("YES");
  comboBox_6->addItem("NO");
  lineEdit_7 = new QLineEdit();
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
  //lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
  lineEdit_7->setPlaceholderText("Optional");
  //lineEdit_8->setPlaceholderText("Optional");
  lineEdit_9->setPlaceholderText("Optional");
  //lineEdit_10->setPlaceholderText("Optional");
  lineEdit_11->setPlaceholderText("Optional");
  lineEdit_12->setPlaceholderText("Optional");
  lineEdit_13->setPlaceholderText("Optional");
  lineEdit_14->setPlaceholderText("Optional");
  lineEdit_15->setPlaceholderText("Optional");

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
  HBoxLayout_6->addWidget(comboBox_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
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
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

StepsModifyCoupledTemperatureDisplacementPanel::~StepsModifyCoupledTemperatureDisplacementPanel()
{}

void StepsModifyCoupledTemperatureDisplacementPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("comboBox_3", comboBox_3->currentText());
  changed = changed || fieldChanged("comboBox_4", comboBox_4->currentText());
  changed = changed || fieldChanged("lineEdit_5", lineEdit_5->text());
  changed = changed || fieldChanged("comboBox_6", comboBox_6->currentText());
  changed = changed || fieldChanged("lineEdit_7", lineEdit_7->text());
  changed = changed || fieldChanged("comboBox_8", comboBox_8->currentText());
  changed = changed || fieldChanged("lineEdit_9", lineEdit_9->text());
  changed = changed || fieldChanged("comboBox_10", comboBox_10->currentText());
  changed = changed || fieldChanged("lineEdit_11", lineEdit_11->text());
  changed = changed || fieldChanged("lineEdit_12", lineEdit_12->text());
  changed = changed || fieldChanged("lineEdit_13", lineEdit_13->text());
  changed = changed || fieldChanged("lineEdit_14", lineEdit_14->text());
  changed = changed || fieldChanged("lineEdit_15", lineEdit_15->text());
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
    PRINT_ERROR("An empty value cannot clear Alpha; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_7->isEnabled() && !originalValues.value("lineEdit_7").isEmpty() && lineEdit_7->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear DELTMX; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_9->isEnabled() && !originalValues.value("lineEdit_9").isEmpty() && lineEdit_9->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Total Time\nat Start; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_11->isEnabled() && !originalValues.value("lineEdit_11").isEmpty() && lineEdit_11->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Initial \nTime Increment; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_12->isEnabled() && !originalValues.value("lineEdit_12").isEmpty() && lineEdit_12->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Time Period\nof Step; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_13->isEnabled() && !originalValues.value("lineEdit_13").isEmpty() && lineEdit_13->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Minimum \nTime Increment; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_14->isEnabled() && !originalValues.value("lineEdit_14").isEmpty() && lineEdit_14->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Maximum \nTime Increment; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_15->isEnabled() && !originalValues.value("lineEdit_15").isEmpty() && lineEdit_15->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Initial \nTime Increment CFD; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx modify step " + lineEdit_1->text() + " coupledtemperaturedisplacement");
    if (lineEdit_2->text()!="" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" name \"" + lineEdit_2->text() + "\"");
    }
    if (comboBox_3->currentIndex()==1 && fieldChanged("comboBox_3", comboBox_3->currentText()))
    {
      command.append(" solver pastix");
    }else if (comboBox_3->currentIndex()==2 && fieldChanged("comboBox_3", comboBox_3->currentText()))
    {
      command.append(" solver pardiso");
    }else if (comboBox_3->currentIndex()==3 && fieldChanged("comboBox_3", comboBox_3->currentText()))
    {
      command.append(" solver spooles");
    }
    if (comboBox_4->currentIndex()==1 && fieldChanged("comboBox_4", comboBox_4->currentText()))
    {
      command.append(" direct_yes");
    }else if (comboBox_4->currentIndex()==2 && fieldChanged("comboBox_4", comboBox_4->currentText()))
    {
      command.append(" direct_no");
    }
    if (lineEdit_5->text()!="" && fieldChanged("lineEdit_5", lineEdit_5->text()))
    {
      command.append(" alpha " + lineEdit_5->text());
    }
    if (comboBox_6->currentIndex()==1 && fieldChanged("comboBox_6", comboBox_6->currentText()))
    {
      command.append(" steadystate_yes");
    }else if (comboBox_6->currentIndex()==2 && fieldChanged("comboBox_6", comboBox_6->currentText()))
    {
      command.append(" steadystate_no");
    }
    if (lineEdit_7->text()!="" && fieldChanged("lineEdit_7", lineEdit_7->text()))
    {
      command.append(" deltmx " + lineEdit_7->text());
    }
    if (comboBox_8->currentIndex()==1 && fieldChanged("comboBox_8", comboBox_8->currentText()))
    {
      command.append(" timereset_yes");
    }else if (comboBox_8->currentIndex()==2 && fieldChanged("comboBox_8", comboBox_8->currentText()))
    {
      command.append(" timereset_no");
    }
    if (lineEdit_9->text()!="" && fieldChanged("lineEdit_9", lineEdit_9->text()))
    {
      command.append(" totaltimeatstart " + lineEdit_9->text());
    }
    if (comboBox_10->currentIndex()==1 && fieldChanged("comboBox_10", comboBox_10->currentText()))
    {
      command.append(" compressible_yes");
    }else if (comboBox_10->currentIndex()==2 && fieldChanged("comboBox_10", comboBox_10->currentText()))
    {
      command.append(" compressible_no");
    }
    if (lineEdit_11->text()!="" && fieldChanged("lineEdit_11", lineEdit_11->text()))
    {
      command.append(" initialtimeincrement " + lineEdit_11->text());
    }
    if (lineEdit_12->text()!="" && fieldChanged("lineEdit_12", lineEdit_12->text()))
    {
      command.append(" timeperiodofstep " + lineEdit_12->text());
    }
    if (lineEdit_13->text()!="" && fieldChanged("lineEdit_13", lineEdit_13->text()))
    {
      command.append(" minimumtimeincrement " + lineEdit_13->text());
    }
    if (lineEdit_14->text()!="" && fieldChanged("lineEdit_14", lineEdit_14->text()))
    {
      command.append(" maximumtimeincrement " + lineEdit_14->text());
    }
    if (lineEdit_15->text()!="" && fieldChanged("lineEdit_15", lineEdit_15->text()))
    {
      command.append(" initialtimeincrementcfd " + lineEdit_15->text());
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


bool StepsModifyCoupledTemperatureDisplacementPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool StepsModifyCoupledTemperatureDisplacementPanel::setStep(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_3->setCurrentIndex(0);
  comboBox_4->setCurrentIndex(0);
  comboBox_6->setCurrentIndex(0);
  comboBox_8->setCurrentIndex(0);
  comboBox_10->setCurrentIndex(0);
  pushButton_apply->setEnabled(false);
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_5->clear();
  lineEdit_7->clear();
  lineEdit_9->clear();
  lineEdit_11->clear();
  lineEdit_12->clear();
  lineEdit_13->clear();
  lineEdit_14->clear();
  lineEdit_15->clear();
  CalculiXStepsInterface core;
  StepsSettings settings;
  std::string error;

  if (!core.get_step_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Step %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 6)
  {
    PRINT_ERROR("The selected record does not match this Step panel.\n");
    return false;
  }
  lineEdit_1->setText(QString::number(settings.id));
  lineEdit_2->setText(QString::fromStdString(settings.name));
  const QString comboBox_3Value = QString::fromStdString(settings.procedure.at(1));
  int comboBox_3Index = comboBox_3->findText(comboBox_3Value, Qt::MatchFixedString);
  if (comboBox_3Index < 0 && !comboBox_3Value.isEmpty())
  {
    comboBox_3->addItem(comboBox_3Value);
    comboBox_3Index = comboBox_3->count() - 1;
  }
  comboBox_3->setCurrentIndex(qMax(0, comboBox_3Index));
  const QString comboBox_4Value = QString::fromStdString(settings.procedure.at(2));
  int comboBox_4Index = comboBox_4->findText(comboBox_4Value, Qt::MatchFixedString);
  if (comboBox_4Index < 0 && !comboBox_4Value.isEmpty())
  {
    comboBox_4->addItem(comboBox_4Value);
    comboBox_4Index = comboBox_4->count() - 1;
  }
  comboBox_4->setCurrentIndex(qMax(0, comboBox_4Index));
  lineEdit_5->setText(QString::fromStdString(settings.procedure.at(3)));
  const QString comboBox_6Value = QString::fromStdString(settings.procedure.at(4));
  int comboBox_6Index = comboBox_6->findText(comboBox_6Value, Qt::MatchFixedString);
  if (comboBox_6Index < 0 && !comboBox_6Value.isEmpty())
  {
    comboBox_6->addItem(comboBox_6Value);
    comboBox_6Index = comboBox_6->count() - 1;
  }
  comboBox_6->setCurrentIndex(qMax(0, comboBox_6Index));
  lineEdit_7->setText(QString::fromStdString(settings.procedure.at(5)));
  const QString comboBox_8Value = QString::fromStdString(settings.procedure.at(6));
  int comboBox_8Index = comboBox_8->findText(comboBox_8Value, Qt::MatchFixedString);
  if (comboBox_8Index < 0 && !comboBox_8Value.isEmpty())
  {
    comboBox_8->addItem(comboBox_8Value);
    comboBox_8Index = comboBox_8->count() - 1;
  }
  comboBox_8->setCurrentIndex(qMax(0, comboBox_8Index));
  lineEdit_9->setText(QString::fromStdString(settings.procedure.at(7)));
  const QString comboBox_10Value = QString::fromStdString(settings.procedure.at(8));
  int comboBox_10Index = comboBox_10->findText(comboBox_10Value, Qt::MatchFixedString);
  if (comboBox_10Index < 0 && !comboBox_10Value.isEmpty())
  {
    comboBox_10->addItem(comboBox_10Value);
    comboBox_10Index = comboBox_10->count() - 1;
  }
  comboBox_10->setCurrentIndex(qMax(0, comboBox_10Index));
  lineEdit_11->setText(QString::fromStdString(settings.procedure.at(9)));
  lineEdit_12->setText(QString::fromStdString(settings.procedure.at(10)));
  lineEdit_13->setText(QString::fromStdString(settings.procedure.at(11)));
  lineEdit_14->setText(QString::fromStdString(settings.procedure.at(12)));
  lineEdit_15->setText(QString::fromStdString(settings.procedure.at(13)));
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("comboBox_3", comboBox_3->currentText());
  originalValues.insert("comboBox_4", comboBox_4->currentText());
  originalValues.insert("lineEdit_5", lineEdit_5->text());
  originalValues.insert("comboBox_6", comboBox_6->currentText());
  originalValues.insert("lineEdit_7", lineEdit_7->text());
  originalValues.insert("comboBox_8", comboBox_8->currentText());
  originalValues.insert("lineEdit_9", lineEdit_9->text());
  originalValues.insert("comboBox_10", comboBox_10->currentText());
  originalValues.insert("lineEdit_11", lineEdit_11->text());
  originalValues.insert("lineEdit_12", lineEdit_12->text());
  originalValues.insert("lineEdit_13", lineEdit_13->text());
  originalValues.insert("lineEdit_14", lineEdit_14->text());
  originalValues.insert("lineEdit_15", lineEdit_15->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
