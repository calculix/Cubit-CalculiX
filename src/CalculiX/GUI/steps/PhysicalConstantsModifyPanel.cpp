#include "CubitMessage.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "PhysicalConstantsModifyPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


PhysicalConstantsModifyPanel::PhysicalConstantsModifyPanel(QWidget *parent) :
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
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_0->setText("Absolute Zero");
  label_1->setText("Stefan Boltzmann");
  label_2->setText("Newton Gravity");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_2 = new QLineEdit();

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(lineEdit_2);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

PhysicalConstantsModifyPanel::~PhysicalConstantsModifyPanel()
{}

void PhysicalConstantsModifyPanel::on_pushButton_apply_clicked(bool)
{
  QString options;
  if (fieldChanged("lineEdit_0", lineEdit_0->text()))
  {
    if (lineEdit_0->text().trimmed().isEmpty())
    {
      if (loadedEntityId >= 0 && !originalValues.value("lineEdit_0").isEmpty())
      {
        PRINT_ERROR("Use the Delete panel to clear Absolute Zero.\n");
        return;
      }
    }
    else
    {
      options.append(" absolute_zero " + lineEdit_0->text());
    }
  }
  if (fieldChanged("lineEdit_1", lineEdit_1->text()))
  {
    if (lineEdit_1->text().trimmed().isEmpty())
    {
      if (loadedEntityId >= 0 && !originalValues.value("lineEdit_1").isEmpty())
      {
        PRINT_ERROR("Use the Delete panel to clear Stefan Boltzmann.\n");
        return;
      }
    }
    else
    {
      options.append(" stefan_boltzmann " + lineEdit_1->text());
    }
  }
  if (fieldChanged("lineEdit_2", lineEdit_2->text()))
  {
    if (lineEdit_2->text().trimmed().isEmpty())
    {
      if (loadedEntityId >= 0 && !originalValues.value("lineEdit_2").isEmpty())
      {
        PRINT_ERROR("Use the Delete panel to clear Newton Gravity.\n");
        return;
      }
    }
    else
    {
      options.append(" newton_gravity " + lineEdit_2->text());
    }
  }
  if (!options.isEmpty())
  {
    GuiSendCmd::gui_cmd("ccx modify physicalconstants" + options);
  }
}


bool PhysicalConstantsModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool PhysicalConstantsModifyPanel::setPhysicalConstants(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  lineEdit_2->clear();
  CalculiXStepsInterface core;
  PhysicalConstantsSettings settings;
  std::string error;
  if (!core.get_physicalconstants_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load PhysicalConstants: %s\n", error.c_str());
    return false;
  }
  lineEdit_0->setText(QString::fromStdString(settings.absoluteZero));
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  lineEdit_1->setText(QString::fromStdString(settings.stefanBoltzmann));
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  lineEdit_2->setText(QString::fromStdString(settings.newtonGravity));
  originalValues.insert("lineEdit_2", lineEdit_2->text());
  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}


bool PhysicalConstantsModifyPanel::loadPhysicalConstants()
{
  return setPhysicalConstants(0);
}
