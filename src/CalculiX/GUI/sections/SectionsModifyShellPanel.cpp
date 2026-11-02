#include "CubitMessage.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "SectionsModifyShellPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


SectionsModifyShellPanel::SectionsModifyShellPanel(QWidget *parent) :
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
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_0->setText("Section ID");
  label_1->setText("Block ID");
  label_2->setText("Material ID");
  label_3->setText("Orientation ID");
  label_4->setText("Thickness");
  label_5->setText("Offset");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  //lineEdit_0->setPlaceholderText("Optional");
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  lineEdit_4->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
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
  HBoxLayout_5->addWidget(lineEdit_5);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

SectionsModifyShellPanel::~SectionsModifyShellPanel()
{}

void SectionsModifyShellPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
  changed = changed || fieldChanged("lineEdit_4", lineEdit_4->text());
  changed = changed || fieldChanged("lineEdit_5", lineEdit_5->text());
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
    PRINT_ERROR("An empty value cannot clear Orientation ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_4->isEnabled() && !originalValues.value("lineEdit_4").isEmpty() && lineEdit_4->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Thickness; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_5->isEnabled() && !originalValues.value("lineEdit_5").isEmpty() && lineEdit_5->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Offset; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify section shell " + lineEdit_0->text());

    if (lineEdit_1->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" block " + lineEdit_1->text());
    }
    if (lineEdit_2->text()!="" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" material " + lineEdit_2->text());
    }
    if (lineEdit_4->text()!="" && fieldChanged("lineEdit_4", lineEdit_4->text()))
    {
      command.append(" thickness " + lineEdit_4->text());
    }
    if (lineEdit_5->text()!="" && fieldChanged("lineEdit_5", lineEdit_5->text()))
    {
      command.append(" offset " + lineEdit_5->text());
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


bool SectionsModifyShellPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool SectionsModifyShellPanel::setSection(int id)
{
  loadedEntityId = -1;
  originalValues.clear();

  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_3->clear();
  lineEdit_4->clear();
  lineEdit_5->clear();
  CalculiXSectionsInterface core;
  SectionsSettings settings;
  std::string error;

  if (!core.get_section_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Section %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 2)
  {
    PRINT_ERROR("The selected record does not match this Section panel.\n");
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.block));
  lineEdit_2->setText(QString::fromStdString(settings.material));
  lineEdit_3->setText(QString::fromStdString(settings.orientation));
  lineEdit_4->setText(QString::fromStdString(settings.thickness));
  lineEdit_5->setText(QString::fromStdString(settings.offset));
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("lineEdit_3", lineEdit_3->text());
  originalValues.insert("lineEdit_4", lineEdit_4->text());
  originalValues.insert("lineEdit_5", lineEdit_5->text());

  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
