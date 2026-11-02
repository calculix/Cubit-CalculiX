#include "CubitMessage.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "DampingModifyPanel.hpp"
#include <QString>
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


DampingModifyPanel::DampingModifyPanel(QWidget *parent) :
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
  label_0 = new QLabel();
  label_1 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_1->setFixedWidth(labelWidth);
  label_0->setText("Alpha");
  label_1->setText("Beta");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

DampingModifyPanel::~DampingModifyPanel()
{}

void DampingModifyPanel::on_pushButton_apply_clicked(bool)
{
  QString options;
  if (fieldChanged("lineEdit_0", lineEdit_0->text()))
  {
    if (lineEdit_0->text().trimmed().isEmpty())
    {
      if (loadedEntityId >= 0 && !originalValues.value("lineEdit_0").isEmpty())
      {
        PRINT_ERROR("Use the Delete panel to clear Alpha.\n");
        return;
      }
    }
    else
    {
      options.append(" alpha " + lineEdit_0->text());
    }
  }
  if (fieldChanged("lineEdit_1", lineEdit_1->text()))
  {
    if (lineEdit_1->text().trimmed().isEmpty())
    {
      if (loadedEntityId >= 0 && !originalValues.value("lineEdit_1").isEmpty())
      {
        PRINT_ERROR("Use the Delete panel to clear Beta.\n");
        return;
      }
    }
    else
    {
      options.append(" beta " + lineEdit_1->text());
    }
  }
  if (!options.isEmpty())
  {
    GuiSendCmd::gui_cmd("ccx modify damping" + options);
  }
}


bool DampingModifyPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool DampingModifyPanel::setDamping(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  CalculiXStepsInterface core;
  DampingSettings settings;
  std::string error;
  if (!core.get_damping_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Damping: %s\n", error.c_str());
    return false;
  }
  lineEdit_0->setText(QString::fromStdString(settings.alpha));
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  lineEdit_1->setText(QString::fromStdString(settings.beta));
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}


bool DampingModifyPanel::loadDamping()
{
  return setDamping(0);
}
