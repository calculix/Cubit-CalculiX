#include "CubitMessage.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "LoadsTrajectoryBodyHeatfluxSphereModifyPanel.hpp"
#include <string>
#include <vector>
#include <QString>
#include "GUI/common/PanelTable.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"


LoadsTrajectoryBodyHeatfluxSphereModifyPanel::LoadsTrajectoryBodyHeatfluxSphereModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  frame_11 = new QFrame();
  frame_11->setMinimumSize(1,300);
  frame_11->setLineWidth(1);
  frame_11->setMidLineWidth(0);
  frame_11->setFrameStyle(QFrame::Box | QFrame::Raised);
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
  HBoxLayout_8 = new QHBoxLayout();
  HBoxLayout_9 = new QHBoxLayout();
  HBoxLayout_10 = new QHBoxLayout();
  HBoxLayout_11 = new QHBoxLayout(frame_11);
  label_0 = new QLabel();
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
  label_0->setFixedWidth(labelWidth);
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
  label_0->setText("Trajectory ID");
  label_1->setText("Name");
  label_2->setText("Load Type");
  label_3->setText("Curve");
  label_4->setText("Vertex ID");
  label_5->setText("Surface IDs");
  label_6->setText("Direction");
  label_7->setText("Time Begin");
  label_8->setText("Time End");
  label_9->setText("OP");
  label_10->setText("Model Change");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  comboBox_2 = new QComboBox();
  //comboBox_2->addItem("");
  comboBox_2->addItem("BodyHeatfluxSphere");
  //comboBox_2->addItem("Pressure");
  PickWidget_3 = new PickWidget();
  PickWidget_3->setPickType(PickWidget::Curve);
  PickWidget_3->activate();
  PickWidget_4 = new PickWidget();
  PickWidget_4->setPickType(PickWidget::Vertex);
  PickWidget_4->activate();
  PickWidget_5 = new PickWidget();
  PickWidget_5->setPickType(PickWidget::Surface);
  PickWidget_5->activate();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  lineEdit_8 = new QLineEdit();
  comboBox_9 = new QComboBox();
  comboBox_9->addItem("");
  comboBox_9->addItem("mod");
  comboBox_9->addItem("new");
  comboBox_10 = new QComboBox();
  comboBox_10->addItem("");
  comboBox_10->addItem("add");
  comboBox_10->addItem("remove");
  comboBox_10->addItem("off");
  widget_11 =  new PanelTable(nullptr);
  matrix.clear();
  widget_11->update({"Ray Radius","Depth","Magnitude"},matrix);
  widget_11->panel_type = "trajectory";
  widget_11->setMinimumSize(200,160);
  
  lineEdit_1->setPlaceholderText("Optional");
  PickWidget_3->setPlaceholderText("Optional");
  PickWidget_4->setPlaceholderText("Optional - Start Vertex");
  PickWidget_5->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("<x> <y> <z>");
  lineEdit_7->setPlaceholderText("<x> <y> <z>");
  lineEdit_8->setPlaceholderText("<x> <y> <z>");
  
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
  VBoxLayout->addLayout(HBoxLayout_8);
  VBoxLayout->addLayout(HBoxLayout_9);
  VBoxLayout->addLayout(HBoxLayout_10);
  VBoxLayout->addWidget(frame_11);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(comboBox_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(PickWidget_3);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(PickWidget_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(PickWidget_5);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(lineEdit_8);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(comboBox_9);
  HBoxLayout_10->addWidget(comboBox_10);
  HBoxLayout_11->addWidget(widget_11);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  widget_11->show();

  isInitialized = true;
}

LoadsTrajectoryBodyHeatfluxSphereModifyPanel::~LoadsTrajectoryBodyHeatfluxSphereModifyPanel()
{}

void LoadsTrajectoryBodyHeatfluxSphereModifyPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("comboBox_2", comboBox_2->currentText());
  changed = changed || fieldChanged("PickWidget_3", PickWidget_3->text());
  changed = changed || fieldChanged("PickWidget_4", PickWidget_4->text());
  changed = changed || fieldChanged("PickWidget_5", PickWidget_5->text());
  changed = changed || fieldChanged("lineEdit_6", lineEdit_6->text());
  changed = changed || fieldChanged("lineEdit_7", lineEdit_7->text());
  changed = changed || fieldChanged("lineEdit_8", lineEdit_8->text());
  changed = changed || fieldChanged("comboBox_9", comboBox_9->currentText());
  changed = changed || fieldChanged("comboBox_10", comboBox_10->currentText());
  changed = changed || fieldChanged("widget_11", widget_11->snapshot());
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
        setTrajectory(requestedId);
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
  if (loadedEntityId >= 0 && PickWidget_3->isEnabled() && !originalValues.value("PickWidget_3").isEmpty() && PickWidget_3->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Curve; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && PickWidget_4->isEnabled() && !originalValues.value("PickWidget_4").isEmpty() && PickWidget_4->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Vertex ID; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && PickWidget_5->isEnabled() && !originalValues.value("PickWidget_5").isEmpty() && PickWidget_5->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Surface IDs; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_6->isEnabled() && !originalValues.value("lineEdit_6").isEmpty() && lineEdit_6->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Direction; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_7->isEnabled() && !originalValues.value("lineEdit_7").isEmpty() && lineEdit_7->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Time Begin; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_8->isEnabled() && !originalValues.value("lineEdit_8").isEmpty() && lineEdit_8->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear Time End; use the supported reset value.\n");
    return;
  }

  if (loadedEntityId >= 0 && !originalValues.value("widget_11").isEmpty() && widget_11->snapshot().isEmpty())
  {
    PRINT_ERROR("The stored table cannot be cleared with an empty Modify command.\n");
    return;
  }

  QStringList commands;
  QString command = "";
  QString tableError;
  if (!widget_11->readMatrix(matrix, tableError))
  {
    PRINT_ERROR("%s\n", tableError.toStdString().c_str());
    return;
  }

  if (lineEdit_0->text()!="")
  {
    command.append("ccx modify trajectory " + lineEdit_0->text() + " " + comboBox_2->currentText());

    if (PickWidget_3->text()!="" && fieldChanged("PickWidget_3", PickWidget_3->text()))
    {
      command.append(" curve " + PickWidget_3->text());
    }
    if (PickWidget_4->text()!="" && fieldChanged("PickWidget_4", PickWidget_4->text()))
    {
      command.append(" vertex " + PickWidget_4->text());
    }

    std::string pickwidget_text = PickWidget_5->text().toUtf8().data();
    if (pickwidget_text!="")
    {
      std::vector<int> surface_ids = CubitInterface::parse_cubit_list("surface", pickwidget_text);
      command.append(" surface");
      for (size_t i = 0; i < surface_ids.size(); i++)
      {
        command.append(" " + QString::number(surface_ids[i]));
      }
    }
    
    if (lineEdit_6->text()!="" && fieldChanged("lineEdit_6", lineEdit_6->text()))
    {
      command.append(" direction " + lineEdit_6->text());
    }

    if (matrix.size()>0 && fieldChanged("widget_11", widget_11->snapshot()))
    {
      command.append(" magnitude ");
    
      for (size_t i = 0; i < matrix.size(); i++)
      {
        command.append(QString::number(matrix[i][2], 'g', 17) + " ");
      }
    }

    if (lineEdit_7->text()!="" && fieldChanged("lineEdit_7", lineEdit_7->text()))
    {
      command.append(" time_begin " + lineEdit_7->text());
    }

    if (lineEdit_8->text()!="" && fieldChanged("lineEdit_8", lineEdit_8->text()))
    {
      command.append(" time_end " + lineEdit_8->text());
    }

    if (matrix.size()>0 && fieldChanged("widget_11", widget_11->snapshot()))
    {
      command.append(" radius ");
    
      for (size_t i = 0; i < matrix.size(); i++)
      {
        command.append(QString::number(matrix[i][0], 'g', 17) + " ");
      }
    }

    if (matrix.size()>0 && fieldChanged("widget_11", widget_11->snapshot()))
    {
      command.append(" depth ");
    
      for (size_t i = 0; i < matrix.size(); i++)
      {
        command.append(QString::number(matrix[i][1], 'g', 17) + " ");
      }
    }

    if (comboBox_9->currentIndex()!=0 && fieldChanged("comboBox_9", comboBox_9->currentText()))
    {
      command.append(" op " + comboBox_9->currentText());
    }

    if (lineEdit_1->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" + lineEdit_1->text() + "\"");
    }

    if (comboBox_10->currentIndex()!=0 && fieldChanged("comboBox_10", comboBox_10->currentText()))
    {
      if (comboBox_10->currentIndex()==1 && fieldChanged("comboBox_10", comboBox_10->currentText()))
      {
        command.append(" modelchange_add ");
      }
      if (comboBox_10->currentIndex()==2 && fieldChanged("comboBox_10", comboBox_10->currentText()))
      {
        command.append(" modelchange_remove ");
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


bool LoadsTrajectoryBodyHeatfluxSphereModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool LoadsTrajectoryBodyHeatfluxSphereModifyPanel::setTrajectory(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_2->setCurrentIndex(0);
  comboBox_9->setCurrentIndex(0);
  comboBox_10->setCurrentIndex(0);
  widget_11->setRowCount(0);
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  PickWidget_3->clear();
  PickWidget_4->clear();
  PickWidget_5->clear();
  lineEdit_6->clear();
  lineEdit_7->clear();
  lineEdit_8->clear();
  CalculiXLoadsInterface core;
  LoadsTrajectorySettings settings;
  std::string error;

  if (!core.get_trajectory_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Trajectory %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 2)
  {
    PRINT_ERROR("The selected record does not match this Trajectory panel.\n");
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));
  const QString comboBox_2Value = QString::fromStdString(settings.loadType);
  int comboBox_2Index = comboBox_2->findText(comboBox_2Value, Qt::MatchFixedString);
  if (comboBox_2Index < 0 && !comboBox_2Value.isEmpty())
  {
    comboBox_2->addItem(comboBox_2Value);
    comboBox_2Index = comboBox_2->count() - 1;
  }
  comboBox_2->setCurrentIndex(qMax(0, comboBox_2Index));
  PickWidget_3->setText(QString::fromStdString(settings.curve));
  PickWidget_4->setText(QString::fromStdString(settings.vertex));
  PickWidget_5->setText(QString::fromStdString(settings.surfaces));
  lineEdit_6->setText(QString::fromStdString(settings.direction));
  lineEdit_7->setText(QString::fromStdString(settings.timeBegin));
  lineEdit_8->setText(QString::fromStdString(settings.timeEnd));
  const QString comboBox_9Value = QString::fromStdString(settings.operation);
  int comboBox_9Index = comboBox_9->findText(comboBox_9Value, Qt::MatchFixedString);
  if (comboBox_9Index < 0 && !comboBox_9Value.isEmpty())
  {
    comboBox_9->addItem(comboBox_9Value);
    comboBox_9Index = comboBox_9->count() - 1;
  }
  comboBox_9->setCurrentIndex(qMax(0, comboBox_9Index));
  const QString comboBox_10Value = QString::fromStdString(settings.modelChange);
  int comboBox_10Index = comboBox_10->findText(comboBox_10Value, Qt::MatchFixedString);
  if (comboBox_10Index < 0 && !comboBox_10Value.isEmpty())
  {
    comboBox_10->addItem(comboBox_10Value);
    comboBox_10Index = comboBox_10->count() - 1;
  }
  comboBox_10->setCurrentIndex(qMax(0, comboBox_10Index));

  widget_11->update({"Radius", "Depth", "Magnitude"}, settings.samples);
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("comboBox_2", comboBox_2->currentText());
  originalValues.insert("PickWidget_3", PickWidget_3->text());
  originalValues.insert("PickWidget_4", PickWidget_4->text());
  originalValues.insert("PickWidget_5", PickWidget_5->text());
  originalValues.insert("lineEdit_6", lineEdit_6->text());
  originalValues.insert("lineEdit_7", lineEdit_7->text());
  originalValues.insert("lineEdit_8", lineEdit_8->text());
  originalValues.insert("comboBox_9", comboBox_9->currentText());
  originalValues.insert("comboBox_10", comboBox_10->currentText());
  originalValues.insert("widget_11", widget_11->snapshot());
  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
