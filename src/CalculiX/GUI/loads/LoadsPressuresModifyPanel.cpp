#include "CubitMessage.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "LoadsPressuresModifyPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


LoadsPressuresModifyPanel::LoadsPressuresModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if (isInitialized)
  {
    return;
  }

  int labelWidth = 120;

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

  label_0->setText("Pressure ID");
  label_1->setText("Name");
  label_2->setText("Sideset ID");
  label_3->setText("Magnitude");
  label_4->setText("OP");
  label_5->setText("Amplitude ID");
  label_6->setText("Time Delay");

  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();

  comboBox_4 = new QComboBox();
  comboBox_4->addItem("");
  comboBox_4->addItem("mod");
  comboBox_4->addItem("new");

  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();

  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
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
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(comboBox_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this, SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

LoadsPressuresModifyPanel::~LoadsPressuresModifyPanel()
{}

void LoadsPressuresModifyPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
  changed = changed || fieldChanged("comboBox_4", comboBox_4->currentText());
  changed = changed || fieldChanged("lineEdit_5", lineEdit_5->text());
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
        setPressure(requestedId);
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
    PRINT_ERROR("An empty value cannot clear Sideset ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_3->isEnabled() && !originalValues.value("lineEdit_3").isEmpty() && lineEdit_3->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Magnitude; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_5->isEnabled() && !originalValues.value("lineEdit_5").isEmpty() && lineEdit_5->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Amplitude ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_6->isEnabled() && !originalValues.value("lineEdit_6").isEmpty() && lineEdit_6->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Time Delay; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if (!lineEdit_0->text().trimmed().isEmpty())
  {
    command.append("ccx modify pressure " + lineEdit_0->text());

    if (!lineEdit_2->text().trimmed().isEmpty() && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" sideset " + lineEdit_2->text());
    }
    if (!lineEdit_3->text().trimmed().isEmpty() && fieldChanged("lineEdit_3", lineEdit_3->text()))
    {
      command.append(" magnitude " + lineEdit_3->text());
    }
    if (comboBox_4->currentIndex() != 0 && fieldChanged("comboBox_4", comboBox_4->currentText()))
    {
      command.append(" op " + comboBox_4->currentText());
    }
    if (!lineEdit_5->text().trimmed().isEmpty() && fieldChanged("lineEdit_5", lineEdit_5->text()))
    {
      command.append(" amplitude " + lineEdit_5->text());
    }
    if (!lineEdit_6->text().trimmed().isEmpty() && fieldChanged("lineEdit_6", lineEdit_6->text()))
    {
      command.append(" timedelay " + lineEdit_6->text());
    }
    if (!lineEdit_1->text().isEmpty() && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }
  }

  if (command != "")
  {
    commands.push_back(command);
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}


bool LoadsPressuresModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool LoadsPressuresModifyPanel::setPressure(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_4->setCurrentIndex(0);
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_3->clear();
  lineEdit_5->clear();
  lineEdit_6->clear();
  CalculiXLoadsInterface core;
  LoadsPressuresSettings settings;
  std::string error;

  if (!core.get_pressure_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Pressure %d: %s\n", id, error.c_str());
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  lineEdit_2->setText(QString::fromStdString(settings.sideset));
  lineEdit_3->setText(QString::fromStdString(settings.magnitude));
  const QString comboBox_4Value = QString::fromStdString(settings.operation);
  int comboBox_4Index = comboBox_4->findText(comboBox_4Value, Qt::MatchFixedString);
  if (comboBox_4Index < 0 && !comboBox_4Value.isEmpty())
  {
    comboBox_4->addItem(comboBox_4Value);
    comboBox_4Index = comboBox_4->count() - 1;
  }
  comboBox_4->setCurrentIndex(qMax(0, comboBox_4Index));
  lineEdit_5->setText(QString::fromStdString(settings.amplitude));
  lineEdit_6->setText(QString::fromStdString(settings.timeDelay));
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("lineEdit_3", lineEdit_3->text());
  originalValues.insert("comboBox_4", comboBox_4->currentText());
  originalValues.insert("lineEdit_5", lineEdit_5->text());
  originalValues.insert("lineEdit_6", lineEdit_6->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
