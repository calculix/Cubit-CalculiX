#include "ConstraintsCreateEquationFromGroupPanel.hpp"
#include "CalculiXCoreInterface.hpp"
#include "PanelTable.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"


ConstraintsCreateEquationFromGroupPanel::ConstraintsCreateEquationFromGroupPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
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
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout(frame_2);
  label_1 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_1->setText("Name");
  lineEdit_1 = new QLineEdit();

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
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addWidget(frame_2);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(widget_2);
  
  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  widget_2->show();
  
  isInitialized = true;
}

ConstraintsCreateEquationFromGroupPanel::~ConstraintsCreateEquationFromGroupPanel()
{}

void ConstraintsCreateEquationFromGroupPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";
  matrix = widget_2->getMatrix();

  if ((lineEdit_1->text()!="") && (matrix.size()!=0))
  {
    command.append("ccx create constraint equation name \"" +lineEdit_1->text() + "\"");
    
    command.append(" node_dof_coefficient ");
    for (size_t i = 0; i < matrix.size(); i++)
    {
      for (size_t ii = 0; ii < matrix[i].size(); ii++)
      {
        command.append(QString::number(matrix[i][ii]) + " ");
      }
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    lineEdit_1->setText("");
    matrix.clear();
    widget_2->update({"Node ID","DOF","Coefficient"},matrix);
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}
