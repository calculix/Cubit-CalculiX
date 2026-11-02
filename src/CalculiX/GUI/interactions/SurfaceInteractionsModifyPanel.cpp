#include "CubitMessage.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "SurfaceInteractionsModifyPanel.hpp"
#include <QString>
#include "GUI/common/PanelTable.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


SurfaceInteractionsModifyPanel::SurfaceInteractionsModifyPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
  matrix.clear();
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  frame_1 = new QFrame();
  frame_1->setMinimumSize(1,300);
  frame_1->setLineWidth(1);
  frame_1->setMidLineWidth(0);
  frame_1->setFrameStyle(QFrame::Box | QFrame::Raised);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  horizontal_spacer_1 = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  HBoxLayout_frame = new QHBoxLayout(frame_1);
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
  comboBox_1 = new QComboBox();
  comboBox_1->addItem("Exponential");
  comboBox_1->addItem("Linear");
  comboBox_1->addItem("Tabular");
  comboBox_1->addItem("Tied");
  comboBox_1->addItem("Gap Conductance");
  comboBox_1->addItem("Gap Heat Generation");
  comboBox_1->addItem("Friction");
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
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth-50);
  label_5->setFixedWidth(labelWidth-50);
  label_6->setFixedWidth(labelWidth-50);
  label_7->setFixedWidth(labelWidth-50);
  label_8->setFixedWidth(labelWidth-50);
  label_9->setFixedWidth(labelWidth-50);
  label_10->setFixedWidth(labelWidth+10);
  label_11->setFixedWidth(labelWidth+10);
  label_12->setFixedWidth(labelWidth+10);
  label_13->setFixedWidth(labelWidth-50);
  label_14->setFixedWidth(labelWidth-50);
  label_1->setText("Surface \nInteraction ID");
  label_2->setText("Name");
  label_3->setText("Surface Behavior:");
  label_4->setText("c0");
  label_5->setText("p0");
  label_6->setText("slope K");
  label_7->setText("sigmaINF");
  label_8->setText("c0");
  label_9->setText("slope K");
  label_10->setText("heat conversion \nfactor");
  label_11->setText("surface weighting \nfactor");
  label_12->setText("differential \ntangential velocity");
  label_13->setText("mu");
  label_14->setText("lambda");
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();
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
  //lineEdit_1->setPlaceholderText("Optional");
  lineEdit_2->setPlaceholderText("Optional");
  lineEdit_4->setPlaceholderText("Optional");
  lineEdit_5->setPlaceholderText("Optional");
  lineEdit_6->setPlaceholderText("Optional");
  lineEdit_7->setPlaceholderText("Optional");
  lineEdit_8->setPlaceholderText("Optional");
  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addWidget(frame_1);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(comboBox_1);

  widget_1 =  new QWidget();
  VBoxLayout_1 = new QVBoxLayout(widget_1);
  vertical_spacer_1 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_1->addLayout(HBoxLayout_4);
  VBoxLayout_1->addLayout(HBoxLayout_5);
  HBoxLayout_4->addWidget(label_4);
  HBoxLayout_4->addWidget(lineEdit_4);
  HBoxLayout_5->addWidget(label_5);
  HBoxLayout_5->addWidget(lineEdit_5);
  VBoxLayout_1->addItem(vertical_spacer_1);

  widget_2 =  new QWidget();
  VBoxLayout_2 = new QVBoxLayout(widget_2);
  vertical_spacer_2 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_2->addLayout(HBoxLayout_6);
  VBoxLayout_2->addLayout(HBoxLayout_7);
  VBoxLayout_2->addLayout(HBoxLayout_8);
  HBoxLayout_6->addWidget(label_6);
  HBoxLayout_6->addWidget(lineEdit_6);
  HBoxLayout_7->addWidget(label_7);
  HBoxLayout_7->addWidget(lineEdit_7);
  HBoxLayout_8->addWidget(label_8);
  HBoxLayout_8->addWidget(lineEdit_8);
  VBoxLayout_2->addItem(vertical_spacer_2);

  widget_3 =  new PanelTable(nullptr);
  widget_3->update({"pressure","overclosure"},matrix);
  //widget_3->setColumnWidth(0,150);
  //widget_3->setMinimumSize(200,280);

  widget_4 =  new QWidget();
  VBoxLayout_3 = new QVBoxLayout(widget_4);
  vertical_spacer_3 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_3->addLayout(HBoxLayout_9);
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(lineEdit_9);
  VBoxLayout_3->addItem(vertical_spacer_3);

  widget_5 =  new PanelTable(nullptr);
  widget_5->update({"Conductance","Contact pressure","Temperature"},matrix);
  //widget_5->setColumnWidth(1,200);
  //widget_5->setMinimumSize(300,280);

  widget_6 =  new QWidget();
  VBoxLayout_4 = new QVBoxLayout(widget_6);
  vertical_spacer_4 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_4->addLayout(HBoxLayout_10);
  HBoxLayout_10->addWidget(label_10);
  HBoxLayout_10->addWidget(lineEdit_10);
  VBoxLayout_4->addLayout(HBoxLayout_11);
  HBoxLayout_11->addWidget(label_11);
  HBoxLayout_11->addWidget(lineEdit_11);
  VBoxLayout_4->addLayout(HBoxLayout_12);
  HBoxLayout_12->addWidget(label_12);
  HBoxLayout_12->addWidget(lineEdit_12);
  VBoxLayout_4->addItem(vertical_spacer_4);

  widget_7 =  new QWidget();
  VBoxLayout_5 = new QVBoxLayout(widget_7);
  vertical_spacer_5 = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  VBoxLayout_5->addLayout(HBoxLayout_13);
  HBoxLayout_13->addWidget(label_13);
  HBoxLayout_13->addWidget(lineEdit_13);
  VBoxLayout_5->addLayout(HBoxLayout_14);
  HBoxLayout_14->addWidget(label_14);
  HBoxLayout_14->addWidget(lineEdit_14);
  VBoxLayout_5->addItem(vertical_spacer_5);

  HBoxLayout_frame->addWidget(widget_1);
  HBoxLayout_frame->addWidget(widget_2);
  HBoxLayout_frame->addWidget(widget_3);
  HBoxLayout_frame->addWidget(widget_4);
  HBoxLayout_frame->addWidget(widget_5);
  HBoxLayout_frame->addWidget(widget_6);
  HBoxLayout_frame->addWidget(widget_7);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  comboBox_1->setCurrentIndex(0);
  widget_1->show();
  widget_2->hide();
  widget_3->hide();
  widget_4->hide();
  widget_5->hide();
  widget_6->hide();
  widget_7->hide();

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));
  QObject::connect(comboBox_1, SIGNAL(currentIndexChanged(int)),this,  SLOT(comboBox_currentIndexChanged(int)));

  isInitialized = true;
}

SurfaceInteractionsModifyPanel::~SurfaceInteractionsModifyPanel()
{}

void SurfaceInteractionsModifyPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_2", lineEdit_2->text());
  changed = changed || fieldChanged("comboBox_1", comboBox_1->currentText());
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
  changed = changed || fieldChanged("widget_3", widget_3->snapshot());
  changed = changed || fieldChanged("widget_5", widget_5->snapshot());
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
        setSurfaceInteraction(requestedId);
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
  if (loadedEntityId >= 0 && lineEdit_4->isEnabled() && !originalValues.value("lineEdit_4").isEmpty() && lineEdit_4->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear c0; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_5->isEnabled() && !originalValues.value("lineEdit_5").isEmpty() && lineEdit_5->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear p0; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_6->isEnabled() && !originalValues.value("lineEdit_6").isEmpty() && lineEdit_6->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear slope K; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_7->isEnabled() && !originalValues.value("lineEdit_7").isEmpty() && lineEdit_7->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear sigmaINF; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_8->isEnabled() && !originalValues.value("lineEdit_8").isEmpty() && lineEdit_8->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear c0; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_9->isEnabled() && !originalValues.value("lineEdit_9").isEmpty() && lineEdit_9->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear slope K; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_10->isEnabled() && !originalValues.value("lineEdit_10").isEmpty() && lineEdit_10->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear heat conversion \nfactor; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_11->isEnabled() && !originalValues.value("lineEdit_11").isEmpty() && lineEdit_11->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear surface weighting \nfactor; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_12->isEnabled() && !originalValues.value("lineEdit_12").isEmpty() && lineEdit_12->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear differential \ntangential velocity; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_13->isEnabled() && !originalValues.value("lineEdit_13").isEmpty() && lineEdit_13->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear mu; use the supported reset value.\n");
    return;
  }
  if (loadedEntityId >= 0 && lineEdit_14->isEnabled() && !originalValues.value("lineEdit_14").isEmpty() && lineEdit_14->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear lambda; use the supported reset value.\n");
    return;
  }

  if (loadedEntityId >= 0 && !originalValues.value("widget_3").isEmpty() && widget_3->snapshot().isEmpty())
  {
    PRINT_ERROR("The stored table cannot be cleared with an empty Modify command.\n");
    return;
  }
  if (loadedEntityId >= 0 && !originalValues.value("widget_5").isEmpty() && widget_5->snapshot().isEmpty())
  {
    PRINT_ERROR("The stored table cannot be cleared with an empty Modify command.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx modify surfaceinteraction " + lineEdit_1->text());

    if (lineEdit_2->text()!="" && fieldChanged("lineEdit_2", lineEdit_2->text()))
    {
      command.append(" name \"" + lineEdit_2->text() + "\"");
      commands.push_back(command);
      command = "ccx modify surfaceinteraction " + lineEdit_1->text();
    }
    
    if (comboBox_1->currentIndex() == 0 && (
      fieldChanged("comboBox_1", comboBox_1->currentText()) ||
      fieldChanged("lineEdit_4", lineEdit_4->text()) ||
      fieldChanged("lineEdit_5", lineEdit_5->text())))
    {
      command.append(" exponential");
      if (lineEdit_4->text()!="" && fieldChanged("lineEdit_4", lineEdit_4->text()))
      {
        command.append(" c0 " + lineEdit_4->text());
      }
      if (lineEdit_5->text()!="" && fieldChanged("lineEdit_5", lineEdit_5->text()))
      {
        command.append(" p0 " + lineEdit_5->text());
      }
    }else if (comboBox_1->currentIndex() == 1 && (
      fieldChanged("comboBox_1", comboBox_1->currentText()) ||
      fieldChanged("lineEdit_6", lineEdit_6->text()) ||
      fieldChanged("lineEdit_7", lineEdit_7->text()) ||
      fieldChanged("lineEdit_8", lineEdit_8->text())))
    {
      command.append(" linear");
      if (lineEdit_6->text()!="" && fieldChanged("lineEdit_6", lineEdit_6->text()))
      {
        command.append(" slopeK " + lineEdit_6->text());
      }
      if (lineEdit_7->text()!="" && fieldChanged("lineEdit_7", lineEdit_7->text()))
      {
        command.append(" sigmaINF " + lineEdit_7->text());
      }
      if (lineEdit_8->text()!="" && fieldChanged("lineEdit_8", lineEdit_8->text()))
      {
        command.append(" c0 " + lineEdit_8->text());
      }
    }else if (comboBox_1->currentIndex() == 2 && (
      fieldChanged("comboBox_1", comboBox_1->currentText()) ||
      fieldChanged("widget_3", widget_3->snapshot())))
    {
      command.append(" tabular pressure_overclosure ");
      QString tableError;
      if (!widget_3->readMatrix(matrix, tableError))
      {
        PRINT_ERROR("%s\n", tableError.toStdString().c_str());
        return;
      }
      for (size_t i = 0; i < matrix.size(); i++)
      {
        for (size_t ii = 0; ii < matrix[i].size(); ii++)
        {
          command.append(QString::number(matrix[i][ii], 'g', 17) + " ");
        }
      }
    }else if (comboBox_1->currentIndex() == 3 && (
      fieldChanged("comboBox_1", comboBox_1->currentText()) ||
      fieldChanged("lineEdit_9", lineEdit_9->text())))
    {
      command.append(" tied slopeK " + lineEdit_9->text());
    }else if (comboBox_1->currentIndex() == 4 && (
      fieldChanged("comboBox_1", comboBox_1->currentText()) ||
      fieldChanged("widget_5", widget_5->snapshot())))
    {
      command.append(" gapconductance conductance_contactpressure_temperature ");
      QString tableError;
      if (!widget_5->readMatrix(matrix, tableError))
      {
        PRINT_ERROR("%s\n", tableError.toStdString().c_str());
        return;
      }
      for (size_t i = 0; i < matrix.size(); i++)
      {
        for (size_t ii = 0; ii < matrix[i].size(); ii++)
        {
          command.append(QString::number(matrix[i][ii], 'g', 17) + " ");
        }
      }
    }else if (comboBox_1->currentIndex() == 5 && (
      fieldChanged("comboBox_1", comboBox_1->currentText()) ||
      fieldChanged("lineEdit_10", lineEdit_10->text()) ||
      fieldChanged("lineEdit_11", lineEdit_11->text()) ||
      fieldChanged("lineEdit_12", lineEdit_12->text())))
    { 
      command.append(" gapheatgeneration");
      if (lineEdit_10->text()!="" && fieldChanged("lineEdit_10", lineEdit_10->text()))
      {
        command.append(" heat_conversion_factor " + lineEdit_10->text());
      }
      if (lineEdit_11->text()!="" && fieldChanged("lineEdit_11", lineEdit_11->text()))
      {
        command.append(" surface_weighing_factor " + lineEdit_11->text());
      }
      if (lineEdit_12->text()!="" && fieldChanged("lineEdit_12", lineEdit_12->text()))
      {
        command.append(" differential_tangential_velocity " + lineEdit_12->text());
      }
    }else if (comboBox_1->currentIndex() == 6 && (
      fieldChanged("comboBox_1", comboBox_1->currentText()) ||
      fieldChanged("lineEdit_13", lineEdit_13->text()) ||
      fieldChanged("lineEdit_14", lineEdit_14->text())))
    { 
      command.append(" friction");
      if (lineEdit_13->text()!="" && fieldChanged("lineEdit_13", lineEdit_13->text()))
      {
        command.append(" mu " + lineEdit_13->text());
      }
      if (lineEdit_14->text()!="" && fieldChanged("lineEdit_14", lineEdit_14->text()))
      {
        command.append(" lambda " + lineEdit_14->text());
      }
    }
  }
  
  if (!command.isEmpty() && command != "ccx modify surfaceinteraction " + lineEdit_1->text())
  {
    commands.push_back(command);
  
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}


void SurfaceInteractionsModifyPanel::comboBox_currentIndexChanged(int index)
{
  widget_1->hide();
  widget_2->hide();
  widget_3->hide();
  widget_4->hide();
  widget_5->hide();
  widget_6->hide();
  widget_7->hide();
  if (index==0)
  {
    widget_1->show();
  }else if (index==1)
  {
    widget_2->show();
  }else if (index==2)
  {
    widget_3->show();
  }else if (index==3)
  {
    widget_4->show();
  }else if (index==4){
    widget_5->show();
  }else if (index==5){
    widget_6->show();
  }else if (index==6){
    widget_7->show();
  }  
}


bool SurfaceInteractionsModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool SurfaceInteractionsModifyPanel::setSurfaceInteraction(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  comboBox_1->setCurrentIndex(0);
  widget_3->setRowCount(0);
  widget_5->setRowCount(0);
  pushButton_apply->setEnabled(false);
  lineEdit_1->clear();
  lineEdit_2->clear();
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
  CalculiXInteractionsInterface core;
  SurfaceInteractionsSettings settings;
  std::string error;

  if (!core.get_surfaceinteraction_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load SurfaceInteraction %d: %s\n", id, error.c_str());
    return false;
  }
  lineEdit_1->setText(QString::number(settings.id));
  lineEdit_2->setText(QString::fromStdString(settings.name));
  const QString comboBox_1Value = QString::fromStdString(settings.behavior);
  int comboBox_1Index = comboBox_1->findText(comboBox_1Value, Qt::MatchFixedString);
  if (comboBox_1Index < 0 && !comboBox_1Value.isEmpty())
  {
    comboBox_1->addItem(comboBox_1Value);
    comboBox_1Index = comboBox_1->count() - 1;
  }
  comboBox_1->setCurrentIndex(qMax(0, comboBox_1Index));
  lineEdit_4->setText(QString::fromStdString(settings.c0));
  lineEdit_5->setText(QString::fromStdString(settings.p0));
  lineEdit_6->setText(QString::fromStdString(settings.slope));
  lineEdit_7->setText(QString::fromStdString(settings.sigma));
  lineEdit_8->setText(QString::fromStdString(settings.linearC0));
  lineEdit_9->setText(QString::fromStdString(settings.tiedSlope));
  lineEdit_10->setText(QString::fromStdString(settings.conversion));
  lineEdit_11->setText(QString::fromStdString(settings.weight));
  lineEdit_12->setText(QString::fromStdString(settings.velocity));
  lineEdit_13->setText(QString::fromStdString(settings.mu));
  lineEdit_14->setText(QString::fromStdString(settings.lambdaValue));

  widget_3->update({"pressure", "overclosure"}, settings.pressureOverclosure);
  widget_5->update({"Conductance", "Contact pressure", "Temperature"}, settings.conductance);
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  originalValues.insert("comboBox_1", comboBox_1->currentText());
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
  originalValues.insert("widget_3", widget_3->snapshot());
  originalValues.insert("widget_5", widget_5->snapshot());
  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
