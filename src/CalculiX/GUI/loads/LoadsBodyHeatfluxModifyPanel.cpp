#include "CubitMessage.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "LoadsBodyHeatfluxModifyPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


LoadsBodyHeatfluxModifyPanel::LoadsBodyHeatfluxModifyPanel(QWidget *parent) :
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
  HBoxLayout_7 = new QHBoxLayout();
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_0->setText("Body Heatflux ID");
  label_1->setText("Name");
  label_2->setText("Magnitude");
  label_3->setText("Type");
  label_4->setText("IDs");
  label_5->setText("OP");
  label_6->setText("Amplitude ID");
  label_7->setText("Time Delay");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  comboBox_3 = new QComboBox();
  comboBox_3->addItem("");
  comboBox_3->addItem("Block");
  comboBox_3->addItem("Element");
  lineEdit_4 = new QLineEdit();
  comboBox_5 = new QComboBox();
  comboBox_5->addItem("");
  comboBox_5->addItem("mod");
  comboBox_5->addItem("new");
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("Optional");
  lineEdit_7->setPlaceholderText("Optional");
  
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
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(comboBox_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
 
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
  
}

LoadsBodyHeatfluxModifyPanel::~LoadsBodyHeatfluxModifyPanel()
{}

void LoadsBodyHeatfluxModifyPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("comboBox_3", comboBox_3->currentText());
  changed = changed || fieldChanged("lineEdit_4", lineEdit_4->text());
  changed = changed || fieldChanged("comboBox_5", comboBox_5->currentText());
  changed = changed || fieldChanged("lineEdit_6", lineEdit_6->text());
  changed = changed || fieldChanged("lineEdit_7", lineEdit_7->text());
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
        setBodyHeatflux(requestedId);
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
    PRINT_ERROR("An empty value cannot clear Magnitude; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_4->isEnabled() && !originalValues.value("lineEdit_4").isEmpty() && lineEdit_4->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear IDs; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_6->isEnabled() && !originalValues.value("lineEdit_6").isEmpty() && lineEdit_6->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Amplitude ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_7->isEnabled() && !originalValues.value("lineEdit_7").isEmpty() && lineEdit_7->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Time Delay; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";
//ccx modify bodyheatflux <bodyheatflux_id> [magnitude <magnitude_value>] [{block|element} <<element_ids>...] [op {mod | new}] [amplitude <amplitude id>] [timedelay <timedelay>] [name <name>] 
  if (lineEdit_0->text()!="")
  {
    command.append("ccx modify bodyheatflux " + lineEdit_0->text());
     
    if (lineEdit_2->text()!="" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" magnitude " +lineEdit_2->text());
    }

    if (!lineEdit_4->text().trimmed().isEmpty() &&
        (fieldChanged("comboBox_3", comboBox_3->currentText()) || fieldChanged("lineEdit_4", lineEdit_4->text())))
    {
      command.append(comboBox_3->currentIndex() == 0 ? " block " : " element ");
      command.append(lineEdit_4->text());
    }

    if (comboBox_5->currentIndex()!=0 && fieldChanged("comboBox_5", comboBox_5->currentText()))
    {
      command.append(" op " + comboBox_5->currentText());
    }

    if (lineEdit_6->text()!="" && fieldChanged("lineEdit_6", lineEdit_6->text()))
    {
      command.append(" amplitude " +lineEdit_6->text());
    }
    if (lineEdit_7->text()!="" && fieldChanged("lineEdit_7", lineEdit_7->text()))
    {
      command.append(" timedelay " +lineEdit_7->text());
    }

    if (lineEdit_1->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" +lineEdit_1->text() + "\"");
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


bool LoadsBodyHeatfluxModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool LoadsBodyHeatfluxModifyPanel::setBodyHeatflux(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_3->setCurrentIndex(0);
  comboBox_5->setCurrentIndex(0);
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_4->clear();
  lineEdit_6->clear();
  lineEdit_7->clear();
  CalculiXLoadsInterface core;
  LoadsBodyHeatfluxSettings settings;
  std::string error;

  if (!core.get_bodyheatflux_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load BodyHeatflux %d: %s\n", id, error.c_str());
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  lineEdit_2->setText(QString::fromStdString(settings.magnitude));
  const QString comboBox_3Value = QString::fromStdString(settings.entityType);
  int comboBox_3Index = comboBox_3->findText(comboBox_3Value, Qt::MatchFixedString);
  if (comboBox_3Index < 0 && !comboBox_3Value.isEmpty())
  {
    comboBox_3->addItem(comboBox_3Value);
    comboBox_3Index = comboBox_3->count() - 1;
  }
  comboBox_3->setCurrentIndex(qMax(0, comboBox_3Index));
  lineEdit_4->setText(QString::fromStdString(settings.ids));
  const QString comboBox_5Value = QString::fromStdString(settings.operation);
  int comboBox_5Index = comboBox_5->findText(comboBox_5Value, Qt::MatchFixedString);
  if (comboBox_5Index < 0 && !comboBox_5Value.isEmpty())
  {
    comboBox_5->addItem(comboBox_5Value);
    comboBox_5Index = comboBox_5->count() - 1;
  }
  comboBox_5->setCurrentIndex(qMax(0, comboBox_5Index));
  lineEdit_6->setText(QString::fromStdString(settings.amplitude));
  lineEdit_7->setText(QString::fromStdString(settings.timeDelay));
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("comboBox_3", comboBox_3->currentText());
  originalValues.insert("lineEdit_4", lineEdit_4->text());
  originalValues.insert("comboBox_5", comboBox_5->currentText());
  originalValues.insert("lineEdit_6", lineEdit_6->text());
  originalValues.insert("lineEdit_7", lineEdit_7->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
