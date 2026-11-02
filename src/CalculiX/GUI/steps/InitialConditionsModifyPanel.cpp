#include "CubitMessage.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "InitialConditionsModifyPanel.hpp"
#include <QString>
#include "GUI/common/PanelTable.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


InitialConditionsModifyPanel::InitialConditionsModifyPanel(QWidget *parent) :
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
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_1->setText("Initial Condition ID");
  label_2->setText("Type");
  label_3->setText("BC Id");
  lineEdit_1 = new QLineEdit();
  comboBox_2 = new QComboBox();
  comboBox_2->addItem("Displacement");
  comboBox_2->addItem("Temperature");
  comboBox_2->addItem("Stress");
  lineEdit_3 = new QLineEdit();
  //lineEdit_1->setPlaceholderText("Optional");
  //lineEdit_2->setPlaceholderText("Optional");

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(comboBox_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(lineEdit_3);
    
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  stressInfo = new QLabel("Stress values are read-only: no stress modify command is available.", this);
  stressInfo->setWordWrap(true);
  blockStress = new PanelTable(this);
  elementStress = new PanelTable(this);
  blockStress->setEditTriggers(QAbstractItemView::NoEditTriggers);
  elementStress->setEditTriggers(QAbstractItemView::NoEditTriggers);
  VBoxLayout->insertWidget(3, stressInfo);
  VBoxLayout->insertWidget(4, blockStress);
  VBoxLayout->insertWidget(5, elementStress);
  stressInfo->hide();
  blockStress->hide();
  elementStress->hide();
  isInitialized = true;
}

InitialConditionsModifyPanel::~InitialConditionsModifyPanel()
{}

void InitialConditionsModifyPanel::on_pushButton_apply_clicked(bool)
{
  if (comboBox_2->currentText() == "Stress")
  {
    PRINT_ERROR("Stress initial conditions have no modify command.\n");
    return;
  }

  bool changed = false;
  changed = changed || fieldChanged("comboBox_2", comboBox_2->currentText());
  changed = changed || fieldChanged("lineEdit_3", lineEdit_3->text());
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
        setInitialCondition(requestedId);
      }
      else
      {
        PRINT_ERROR("Select a single entity ID before reloading this panel.\n");
      }

      return;
    }
  }

  if (loadedEntityId >= 0 && lineEdit_3->isEnabled() && !originalValues.value("lineEdit_3").isEmpty() && lineEdit_3->text().trimmed().isEmpty())
  {
    PRINT_ERROR("An empty value cannot clear BC Id; use the supported reset value.\n");
    return;
  }

  QStringList commands;
  QString command = "";

  if ((lineEdit_1->text()!=""))
  {
    command.append("ccx modify initialcondition " + lineEdit_1->text());
       
    if (comboBox_2->currentIndex()==0)
    {
      command.append(" displacement");
    }else if (comboBox_2->currentIndex()==1)
    {
      command.append(" temperature");
    }
    if (lineEdit_3->text()!="" && fieldChanged("lineEdit_3", lineEdit_3->text()))
    {
      command.append(" bc_id " + lineEdit_3->text());
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


bool InitialConditionsModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool InitialConditionsModifyPanel::setInitialCondition(int id)
{
  loadedEntityId = -1;
  stressInfo->hide();
  blockStress->hide();
  elementStress->hide();
  blockStress->setRowCount(0);
  elementStress->setRowCount(0);
  originalValues.clear();
  comboBox_2->setCurrentIndex(0);
  pushButton_apply->setEnabled(false);
  lineEdit_1->clear();
  lineEdit_3->clear();
  CalculiXStepsInterface core;
  InitialConditionsSettings settings;
  std::string error;

  if (!core.get_initialcondition_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load InitialCondition %d: %s\n", id, error.c_str());
    return false;
  }
  lineEdit_1->setText(QString::number(settings.id));
  const QString comboBox_2Value = QString::fromStdString(settings.conditionType);
  int comboBox_2Index = comboBox_2->findText(comboBox_2Value, Qt::MatchFixedString);
  if (comboBox_2Index < 0 && !comboBox_2Value.isEmpty())
  {
    comboBox_2->addItem(comboBox_2Value);
    comboBox_2Index = comboBox_2->count() - 1;
  }
  comboBox_2->setCurrentIndex(qMax(0, comboBox_2Index));
  lineEdit_3->setText(QString::fromStdString(settings.bcId));

  lineEdit_3->setEnabled(settings.type != 3);
  if (settings.type == 3)
  {
    PRINT_INFO("Stress initial conditions have no modify command; stored stresses are read-only.\n");
    pushButton_apply->setEnabled(false);
  }
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("comboBox_2", comboBox_2->currentText());
  originalValues.insert("lineEdit_3", lineEdit_3->text());

  blockStress->update({"Block", "Sxx", "Syy", "Szz", "Sxy", "Sxz", "Syz"}, settings.blockStress);
  elementStress->update({"Element", "IP", "Sxx", "Syy", "Szz", "Sxy", "Sxz", "Syz"}, settings.elementStress);
  stressInfo->setVisible(settings.type == 3);
  blockStress->setVisible(settings.type == 3);
  elementStress->setVisible(settings.type == 3);
  loadedEntityId = id;
  pushButton_apply->setEnabled(settings.type != 3);
  return true;
}
