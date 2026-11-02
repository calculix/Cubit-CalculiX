#include "CubitMessage.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "ConstraintsModifyEquationPanel.hpp"
#include <QString>
#include "GUI/common/PanelTable.hpp"
#include "GUI/application/GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


ConstraintsModifyEquationPanel::ConstraintsModifyEquationPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
  {
    return;
  }
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  frame_2 = new QFrame();
  frame_2->setMinimumSize(1,300);
  frame_2->setLineWidth(1);
  frame_2->setMidLineWidth(0);
  frame_2->setFrameStyle(QFrame::Box | QFrame::Raised);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_0 = new QHBoxLayout();
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout(frame_2);
  label_0 = new QLabel();
  label_0->setFixedWidth(labelWidth);
  label_0->setText("Equation ID");
  label_1 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_1->setText("Name");
  lineEdit_0 = new QLineEdit();
  lineEdit_1 = new QLineEdit();
  lineEdit_1->setPlaceholderText("Optional");

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  widget_2 =  new PanelTable(nullptr);
  matrix.clear();
  widget_2->update({"Node ID","DOF","Coefficient"},matrix);
  widget_2->panel_type = "equation";
  widget_2->setMinimumSize(200,160);

  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_0);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addWidget(frame_2);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_0->addWidget(label_0);
  HBoxLayout_0->addWidget(lineEdit_0);
  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(widget_2);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  widget_2->show();
  
  isInitialized = true;
}

ConstraintsModifyEquationPanel::~ConstraintsModifyEquationPanel()
{}

void ConstraintsModifyEquationPanel::on_pushButton_apply_clicked(bool)
{
  bool changed = false;
  changed = changed || fieldChanged("lineEdit_1", lineEdit_1->text());
  changed = changed || fieldChanged("widget_2", widget_2->snapshot());
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
        setConstraint(requestedId);
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

  if (loadedEntityId >= 0 && !originalValues.value("widget_2").isEmpty() && widget_2->snapshot().isEmpty())
  {
    PRINT_ERROR("The stored table cannot be cleared with an empty Modify command.\n");
    return;
  }

  QStringList commands;
  QString command = "";
  QString tableError;
  if (!widget_2->readMatrix(matrix, tableError))
  {
    PRINT_ERROR("%s\n", tableError.toStdString().c_str());
    return;
  }

  if ((lineEdit_0->text()!=""))
  {
    command.append("ccx modify constraint equation "+ lineEdit_0->text());
    
    if (lineEdit_1->text()!="" && fieldChanged("lineEdit_1", lineEdit_1->text()))
    {
      command.append(" name \"" +lineEdit_1->text() + "\"");
    }
    if (matrix.size()!=0 && fieldChanged("widget_2", widget_2->snapshot()))
    {
      command.append(" node_dof_coefficient ");
      for (size_t i = 0; i < matrix.size(); i++)
      {
        for (size_t ii = 0; ii < matrix[i].size(); ii++)
        {
          command.append(QString::number(matrix[i][ii], 'g', 17) + " ");
        }
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


bool ConstraintsModifyEquationPanel::fieldChanged(const QString& field, const QString& value) const
{
  return loadedEntityId < 0 || originalValues.value(field) != value;
}

bool ConstraintsModifyEquationPanel::setConstraint(int id)
{
  loadedEntityId = -1;
  originalValues.clear();
  widget_2->setRowCount(0);
  pushButton_apply->setEnabled(false);
  lineEdit_0->clear();
  lineEdit_1->clear();
  CalculiXInteractionsInterface core;
  ConstraintsSettings settings;
  std::string error;

  if (!core.get_constraint_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Constraint %d: %s\n", id, error.c_str());
    return false;
  }

  if (settings.type != 3)
  {
    PRINT_ERROR("The selected record does not match this Constraint panel.\n");
    return false;
  }
  lineEdit_0->setText(QString::number(settings.id));
  lineEdit_1->setText(QString::fromStdString(settings.name));

  widget_2->update({"Node ID", "DOF", "Coefficient"}, settings.terms);
  originalValues.insert("lineEdit_0", lineEdit_0->text());
  originalValues.insert("lineEdit_1", lineEdit_1->text());
  originalValues.insert("widget_2", widget_2->snapshot());
  loadedEntityId = id;
  pushButton_apply->setEnabled(true);
  return true;
}
