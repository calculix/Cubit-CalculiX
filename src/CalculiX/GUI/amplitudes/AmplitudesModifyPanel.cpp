#include "CubitMessage.hpp"
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "AmplitudesModifyPanel.hpp"
#include <QString>
#include "GUI/common/PanelTable.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


AmplitudesModifyPanel::AmplitudesModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  frame_5 = new QFrame();
  frame_5->setMinimumSize(1,300);
  frame_5->setLineWidth(1);
  frame_5->setMidLineWidth(0);
  frame_5->setFrameStyle(QFrame::Box | QFrame::Raised);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_0 = new QHBoxLayout();
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout(frame_5);
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
  label_0->setText("Amplitude ID");
  label_1->setText("Name");
  label_2->setText("Shift x");
  label_3->setText("Shift y");
  label_4->setText("Time=TotalTime");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
  lineEdit_3 = new QLineEdit();
  checkBox_4 = new QCheckBox();
  lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_3->setPlaceholderText("Optional");
  

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  widget_5 =  new PanelTable(nullptr);
  matrix.clear();
  widget_5->update({"Time","Amplitude"},matrix);
  //widget_3->setColumnWidth(0,150);
  widget_5->panel_type = "amplitude";
  widget_5->setMinimumSize(200,160);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addWidget(frame_5);
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
  HBoxLayout_4->addWidget(checkBox_4);
  HBoxLayout_5->addWidget(widget_5);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  widget_5->show();
  checkBox_4->setChecked(false);

  isInitialized = true;
}

AmplitudesModifyPanel::~AmplitudesModifyPanel()
{}

void AmplitudesModifyPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
  changed = changed || fieldChanged("checkBox_4", (checkBox_4->isChecked() ? QStringLiteral("YES") : QStringLiteral("NO")));
  changed = changed || fieldChanged("widget_5", widget_5->snapshot());
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
        setAmplitude(requestedId);
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
    PRINT_ERROR("An empty value cannot clear Shift x; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_3->isEnabled() && !originalValues.value("lineEdit_3").isEmpty() && lineEdit_3->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Shift y; use the supported reset value.\n");
    return;
  }

  if (loadedEntityId >= 0 && !originalValues.value("widget_5").isEmpty() && widget_5->snapshot().isEmpty())
  {
    PRINT_ERROR("The stored table cannot be cleared with an empty Modify command.\n");
    return;
  }

  QStringList commands;
  QString command = "";
  QString tableError;
  if (!widget_5->readMatrix(matrix, tableError))
  {
    PRINT_ERROR("%s\n", tableError.toStdString().c_str());
    return;
  }

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify amplitude "+ lineEdit_0->text());
    
    if (lineEdit_1->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" +lineEdit_1->text() + "\"");
    }
    if (matrix.size()!=0 && fieldChanged("widget_5", widget_5->snapshot()))
    {
      command.append(" time_amplitude ");
      for (size_t i = 0; i < matrix.size(); i++)
      {
        for (size_t ii = 0; ii < matrix[i].size(); ii++)
        {
          command.append(QString::number(matrix[i][ii], 'g', 17) + " ");
        }
      }
    }
    if (lineEdit_2->text()!="" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" shiftx " +lineEdit_2->text());
    }
    if (lineEdit_3->text()!="" && fieldChanged("lineEdit_3", lineEdit_3->text()))
    {
      command.append(" shifty " +lineEdit_3->text());
    }
    if (fieldChanged("checkBox_4", checkBox_4->isChecked() ? "YES" : "NO"))
    {
      if (checkBox_4->isChecked())
      {
        command.append(" totaltime_yes");
      }
      else
      {
        command.append(" totaltime_no");
      }
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


bool AmplitudesModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool AmplitudesModifyPanel::setAmplitude(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  checkBox_4->setChecked(false);
  widget_5->setRowCount(0);
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  lineEdit_3->clear();
  CalculiXAmplitudesInterface core;
  AmplitudesSettings settings;
  std::string error;

  if (!core.get_amplitude_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Amplitude %d: %s\n", id, error.c_str());
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  lineEdit_2->setText(QString::fromStdString(settings.shiftX));
  lineEdit_3->setText(QString::fromStdString(settings.shiftY));
  checkBox_4->setChecked(settings.totalTime == "YES");

  widget_5->update({"Time", "Amplitude"}, settings.points);
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("lineEdit_3", lineEdit_3->text());
  originalValues.insert("checkBox_4", (checkBox_4->isChecked() ? QStringLiteral("YES") : QStringLiteral("NO")));
  originalValues.insert("widget_5", widget_5->snapshot());
  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
