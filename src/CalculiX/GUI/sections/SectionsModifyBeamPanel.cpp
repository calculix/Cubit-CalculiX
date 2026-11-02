#include "CubitMessage.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "SectionsModifyBeamPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


SectionsModifyBeamPanel::SectionsModifyBeamPanel(QWidget *parent) :
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
  HBoxLayout_beam_type = new QHBoxLayout();
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
  label_0 = new QLabel();
  label_beam_type = new QLabel();
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
  label_0->setFixedWidth(labelWidth);
  label_beam_type->setFixedWidth(labelWidth);
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
  label_0->setText("Section ID");
  label_beam_type->setText("Beam Type");
  label_1->setText("Block ID");
  label_2->setText("Material ID");
  label_3->setText("Parameter 1");
  label_4->setText("Parameter 2");
  label_5->setText("Parameter 3");
  label_6->setText("Parameter 4");
  label_7->setText("Parameter 5");
  label_8->setText("Parameter 6");
  label_9->setText("x");
  label_10->setText("y");
  label_11->setText("z");
  label_12->setText("Orientation ID");
  label_13->setText("Offset 1");
  label_14->setText("Offset 2");
  lineEdit_0 = new QLineEdit();
  beam_type = new QComboBox();
  beam_type->addItem("");
  beam_type->addItem("RECT");
  beam_type->addItem("CIRC");
  beam_type->addItem("PIPE");
  beam_type->addItem("BOX");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  lineEdit_8 = new QLineEdit();
  lineEdit_9 = new QLineEdit();
  lineEdit_10 = new QLineEdit();
  lineEdit_11 = new QLineEdit();
  lineEdit_12 = new QLineEdit();
  lineEdit_13 = new QLineEdit();
  lineEdit_14 = new QLineEdit();
  //lineEdit_0->setPlaceholderText("Optional");
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  lineEdit_4->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("only for BOX");
  lineEdit_6->setPlaceholderText("only for BOX");
  lineEdit_7->setPlaceholderText("only for BOX");
  lineEdit_8->setPlaceholderText("only for BOX");
  lineEdit_9->setPlaceholderText("Optional");
  lineEdit_10->setPlaceholderText("Optional");
  lineEdit_11->setPlaceholderText("Optional");
  lineEdit_12->setPlaceholderText("Optional");
  lineEdit_13->setPlaceholderText("Optional");
  lineEdit_14->setPlaceholderText("Optional");
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_beam_type);
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
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_beam_type->addWidget(label_beam_type);
  HBoxLayout_beam_type->addWidget(beam_type);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(lineEdit_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(lineEdit_9);
  HBoxLayout_10->addWidget(label_10);
  HBoxLayout_10->addWidget(lineEdit_10);
  HBoxLayout_11->addWidget(label_11);
  HBoxLayout_11->addWidget(lineEdit_11);
  HBoxLayout_12->addWidget(label_12);
  HBoxLayout_12->addWidget(lineEdit_12);
  HBoxLayout_13->addWidget(label_13);
  HBoxLayout_13->addWidget(lineEdit_13);
  HBoxLayout_14->addWidget(label_14);
  HBoxLayout_14->addWidget(lineEdit_14);
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

SectionsModifyBeamPanel::~SectionsModifyBeamPanel()
{}

void SectionsModifyBeamPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("beam_type", beam_type->currentText());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
  changed = changed || fieldChanged("lineEdit_4", lineEdit_4->text());
  changed = changed || fieldChanged("lineEdit_5", lineEdit_5->text());
  changed = changed || fieldChanged("lineEdit_6", lineEdit_6->text());
  changed = changed || fieldChanged("lineEdit_7", lineEdit_7->text());
  changed = changed || fieldChanged("lineEdit_8", lineEdit_8->text());
  changed = changed || fieldChanged("lineEdit_9", lineEdit_9->text());
  changed = changed || fieldChanged("lineEdit_10", lineEdit_10->text());
  changed = changed || fieldChanged("lineEdit_11", lineEdit_11->text());
  changed = changed || fieldChanged("lineEdit_12", lineEdit_12->text());
  changed = changed || fieldChanged("lineEdit_13", lineEdit_13->text());
  changed = changed || fieldChanged("lineEdit_14", lineEdit_14->text());
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
        setSection(requestedId);
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
    PRINT_ERROR("An empty value cannot clear Block ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_2->isEnabled() && !originalValues.value("lineEdit_2").isEmpty() && lineEdit_2->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Material ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_3->isEnabled() && !originalValues.value("lineEdit_3").isEmpty() && lineEdit_3->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Parameter 1; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_4->isEnabled() && !originalValues.value("lineEdit_4").isEmpty() && lineEdit_4->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Parameter 2; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_5->isEnabled() && !originalValues.value("lineEdit_5").isEmpty() && lineEdit_5->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Parameter 3; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_6->isEnabled() && !originalValues.value("lineEdit_6").isEmpty() && lineEdit_6->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Parameter 4; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_7->isEnabled() && !originalValues.value("lineEdit_7").isEmpty() && lineEdit_7->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Parameter 5; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_8->isEnabled() && !originalValues.value("lineEdit_8").isEmpty() && lineEdit_8->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Parameter 6; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_9->isEnabled() && !originalValues.value("lineEdit_9").isEmpty() && lineEdit_9->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear x; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_10->isEnabled() && !originalValues.value("lineEdit_10").isEmpty() && lineEdit_10->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear y; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_11->isEnabled() && !originalValues.value("lineEdit_11").isEmpty() && lineEdit_11->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear z; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_12->isEnabled() && !originalValues.value("lineEdit_12").isEmpty() && lineEdit_12->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Orientation ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_13->isEnabled() && !originalValues.value("lineEdit_13").isEmpty() && lineEdit_13->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Offset 1; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_14->isEnabled() && !originalValues.value("lineEdit_14").isEmpty() && lineEdit_14->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Offset 2; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify section beam " + lineEdit_0->text());
    
    
    if (beam_type->currentIndex()==1 && fieldChanged("beam_type", beam_type->currentText()))
    {
      command.append(" beam_type rect ");
    }else if (beam_type->currentIndex()==2 && fieldChanged("beam_type", beam_type->currentText()))
    {
      command.append(" beam_type circ ");
    }else if (beam_type->currentIndex()==3 && fieldChanged("beam_type", beam_type->currentText()))
    {
      command.append(" beam_type pipe ");
    }else if (beam_type->currentIndex()==4 && fieldChanged("beam_type", beam_type->currentText()))
    {
      command.append(" beam_type box ");
    }
    
    if (lineEdit_1->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" block " + lineEdit_1->text());
    }
    if (lineEdit_2->text()!="" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" material " + lineEdit_2->text());
    }
    if (lineEdit_3->text()!="" && fieldChanged("lineEdit_3", lineEdit_3->text()))
    {
      command.append(" parameter1 " + lineEdit_3->text());
    }
    if (lineEdit_4->text()!="" && fieldChanged("lineEdit_4", lineEdit_4->text()))
    {
      command.append(" parameter2 " + lineEdit_4->text());
    }
    if (lineEdit_5->text()!="" && fieldChanged("lineEdit_5", lineEdit_5->text()))
    {
      command.append(" parameter3 " + lineEdit_5->text());
    }
    if (lineEdit_6->text()!="" && fieldChanged("lineEdit_6", lineEdit_6->text()))
    {
      command.append(" parameter4 " + lineEdit_6->text());
    }
    if (lineEdit_7->text()!="" && fieldChanged("lineEdit_7", lineEdit_7->text()))
    {
      command.append(" parameter5 " + lineEdit_7->text());
    }
    if (lineEdit_8->text()!="" && fieldChanged("lineEdit_8", lineEdit_8->text()))
    {
      command.append(" parameter6 " + lineEdit_8->text());
    }
    if (lineEdit_9->text()!="" && fieldChanged("lineEdit_9", lineEdit_9->text()))
    {
      command.append(" x " + lineEdit_9->text());
    }
    if (lineEdit_10->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" y " + lineEdit_10->text());
    }
    if (lineEdit_11->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" z " + lineEdit_11->text());
    }
    if (lineEdit_12->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" orientation " + lineEdit_12->text());
    }
    if (lineEdit_13->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" offset1 " + lineEdit_13->text());
    }
    if (lineEdit_14->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" offset2 " + lineEdit_14->text());
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


bool SectionsModifyBeamPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool SectionsModifyBeamPanel::setSection(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  beam_type->setCurrentIndex(0);
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_3->clear();
  lineEdit_4->clear();
  lineEdit_5->clear();
  lineEdit_6->clear();
  lineEdit_7->clear();
  lineEdit_8->clear();
  lineEdit_9->clear();
  lineEdit_10->clear();
  lineEdit_11->clear();
  lineEdit_12->clear();
  lineEdit_13->clear();
  lineEdit_14->clear();
  CalculiXSectionsInterface core;
  SectionsSettings settings;
  std::string error;

  if (!core.get_section_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Section %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 3)
  {
    PRINT_ERROR("The selected record does not match this Section panel.\n");
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.block));
  lineEdit_2->setText(QString::fromStdString(settings.material));
  const QString beam_typeValue = QString::fromStdString(settings.beamType);
  int beam_typeIndex = beam_type->findText(beam_typeValue, Qt::MatchFixedString);
  if (beam_typeIndex < 0 && !beam_typeValue.isEmpty())
  {
    beam_type->addItem(beam_typeValue);
    beam_typeIndex = beam_type->count() - 1;
  }
  beam_type->setCurrentIndex(qMax(0, beam_typeIndex));
  lineEdit_3->setText(QString::fromStdString(settings.parameter1));
  lineEdit_4->setText(QString::fromStdString(settings.parameter2));
  lineEdit_5->setText(QString::fromStdString(settings.parameter3));
  lineEdit_6->setText(QString::fromStdString(settings.parameter4));
  lineEdit_7->setText(QString::fromStdString(settings.parameter5));
  lineEdit_8->setText(QString::fromStdString(settings.parameter6));
  lineEdit_9->setText(QString::fromStdString(settings.x));
  lineEdit_10->setText(QString::fromStdString(settings.y));
  lineEdit_11->setText(QString::fromStdString(settings.z));
  lineEdit_12->setText(QString::fromStdString(settings.orientation));
  lineEdit_13->setText(QString::fromStdString(settings.offset1));
  lineEdit_14->setText(QString::fromStdString(settings.offset2));
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("beam_type", beam_type->currentText());
  originalValues.insert("lineEdit_3", lineEdit_3->text());
  originalValues.insert("lineEdit_4", lineEdit_4->text());
  originalValues.insert("lineEdit_5", lineEdit_5->text());
  originalValues.insert("lineEdit_6", lineEdit_6->text());
  originalValues.insert("lineEdit_7", lineEdit_7->text());
  originalValues.insert("lineEdit_8", lineEdit_8->text());
  originalValues.insert("lineEdit_9", lineEdit_9->text());
  originalValues.insert("lineEdit_10", lineEdit_10->text());
  originalValues.insert("lineEdit_11", lineEdit_11->text());
  originalValues.insert("lineEdit_12", lineEdit_12->text());
  originalValues.insert("lineEdit_13", lineEdit_13->text());
  originalValues.insert("lineEdit_14", lineEdit_14->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
