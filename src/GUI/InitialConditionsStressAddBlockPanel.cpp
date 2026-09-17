#include "InitialConditionsStressAddBlockPanel.hpp"
#include "PanelTable.hpp"
#include "CalculiXCoreInterface.hpp"
#include "GuiSendCmd.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"


InitialConditionsStressAddBlockPanel::InitialConditionsStressAddBlockPanel(QWidget *parent) :
  QWidget(parent),
  isInitialized(false)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
  int labelWidth = 120;
  //this->setMinimumSize(1000,300);
  GridLayout = new QGridLayout(this);
  VBoxLayout = new QVBoxLayout();
  vertical_spacer = new QSpacerItem(1,1,QSizePolicy::Minimum,QSizePolicy::Expanding);
  HBoxLayout_1 = new QHBoxLayout();
  HBoxLayout_2 = new QHBoxLayout();
  HBoxLayout_3 = new QHBoxLayout();
  HBoxLayout_4 = new QHBoxLayout();
  HBoxLayout_5 = new QHBoxLayout();
  HBoxLayout_6 = new QHBoxLayout();
  HBoxLayout_7 = new QHBoxLayout();
  HBoxLayout_8 = new QHBoxLayout();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_1->setText("Initialcondition ID");
  label_2->setText("Block ID");
  label_3->setText("SXX");
  label_4->setText("SYY");
  label_5->setText("SZZ");
  label_6->setText("SXY");
  label_7->setText("SXZ");
  label_8->setText("SYZ");
  lineEdit_1 = new QLineEdit();
  PickWidget_2 = new PickWidget();
  PickWidget_2->setPickType(PickWidget::Block);
  PickWidget_2->activate();
  lineEdit_3 = new QLineEdit();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  lineEdit_8 = new QLineEdit();

  pushButton_apply = new QPushButton();
  pushButton_apply->setText("Apply");
  HBoxLayout_pushButton_apply = new QHBoxLayout();
  horizontal_spacer_pushButton_apply = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Minimum);
  
  // Layout
  GridLayout->addLayout(VBoxLayout,0,0, Qt::AlignTop);
  VBoxLayout->addLayout(HBoxLayout_1);
  VBoxLayout->addLayout(HBoxLayout_2);
  VBoxLayout->addLayout(HBoxLayout_3);
  VBoxLayout->addLayout(HBoxLayout_4);
  VBoxLayout->addLayout(HBoxLayout_5);
  VBoxLayout->addLayout(HBoxLayout_6);
  VBoxLayout->addLayout(HBoxLayout_7);
  VBoxLayout->addLayout(HBoxLayout_8);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(lineEdit_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(PickWidget_2);
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

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

InitialConditionsStressAddBlockPanel::~InitialConditionsStressAddBlockPanel()
{}

void InitialConditionsStressAddBlockPanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  std::vector<int> block_ids = CubitInterface::parse_cubit_list("block",PickWidget_2->text().toStdString());

  if ((lineEdit_1->text()!="") && (block_ids.size()>0)
      && (lineEdit_3->text()!="") && (lineEdit_4->text()!="")
      && (lineEdit_5->text()!="") && (lineEdit_6->text()!="")
      && (lineEdit_7->text()!="") && (lineEdit_8->text()!=""))
  {
    for (size_t i = 0; i < block_ids.size(); i++)
    {
      command.append("ccx initialcondition " + lineEdit_1->text());
      command.append(" stress add block " + QString::number(block_ids[i]));
      command.append(" sxx " + lineEdit_3->text());
      command.append(" syy " + lineEdit_4->text());
      command.append(" szz " + lineEdit_5->text());
      command.append(" sxy " + lineEdit_6->text());
      command.append(" sxz " + lineEdit_7->text());
      command.append(" syz " + lineEdit_8->text());
      
      commands.push_back(command);
    
    }
  }
  
  if (command.size() > 0)
  {
    lineEdit_1->setText("");
    PickWidget_2->setText("");
    lineEdit_3->setText("");
    lineEdit_4->setText("");
    lineEdit_5->setText("");
    lineEdit_6->setText("");
    lineEdit_7->setText("");
    lineEdit_8->setText("");
  }

  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    //ccx_iface->cmd(commands[int(i)].toStdString());
    GuiSendCmd::gui_cmd(commands[int(i)]);
  }
}
