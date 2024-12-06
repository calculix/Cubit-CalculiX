#include "LoadsTrajectoryCreatePanel.hpp"
#include "CalculiXCoreInterface.hpp"

#include "CubitInterface.hpp"
#include "Broker.hpp"
#include "Claro.hpp"
#include "PickWidget.hpp"


LoadsTrajectoryCreatePanel::LoadsTrajectoryCreatePanel(QWidget *parent) :
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
  HBoxLayout_9 = new QHBoxLayout();
  label_1 = new QLabel();
  label_2 = new QLabel();
  label_3 = new QLabel();
  label_4 = new QLabel();
  label_5 = new QLabel();
  label_6 = new QLabel();
  label_7 = new QLabel();
  label_8 = new QLabel();
  label_9 = new QLabel();
  label_1->setFixedWidth(labelWidth);
  label_2->setFixedWidth(labelWidth);
  label_3->setFixedWidth(labelWidth);
  label_4->setFixedWidth(labelWidth);
  label_5->setFixedWidth(labelWidth);
  label_6->setFixedWidth(labelWidth);
  label_7->setFixedWidth(labelWidth);
  label_8->setFixedWidth(labelWidth);
  label_9->setFixedWidth(labelWidth);
  label_1->setText("Curve");
  label_2->setText("Vertex ID");
  label_3->setText("Surface IDs");
  label_4->setText("Direction");
  label_5->setText("Magnitude");
  label_6->setText("Time Begin");
  label_7->setText("Time End");
  label_8->setText("Radius");
  label_9->setText("OP");
  PickWidget_1 = new PickWidget();
  PickWidget_1->setPickType(PickWidget::Curve);
  PickWidget_1->activate();
  PickWidget_2 = new PickWidget();
  PickWidget_2->setPickType(PickWidget::Vertex);
  PickWidget_2->activate();
  PickWidget_3 = new PickWidget();
  PickWidget_3->setPickType(PickWidget::Surface);
  PickWidget_3->activate();
  lineEdit_4 = new QLineEdit();
  lineEdit_5 = new QLineEdit();
  lineEdit_6 = new QLineEdit();
  lineEdit_7 = new QLineEdit();
  lineEdit_8 = new QLineEdit();
  comboBox_9 = new QComboBox();
  comboBox_9->addItem("");
  comboBox_9->addItem("mod");
  comboBox_9->addItem("new");
  
  PickWidget_2->setPlaceholderText("Start Vertex");
  lineEdit_4->setPlaceholderText("<x> <y> <z>");
  lineEdit_5->setPlaceholderText("Heatflux Magnitude");
  lineEdit_8->setPlaceholderText("Ray Radius");
  
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
  VBoxLayout->addLayout(HBoxLayout_9);
  VBoxLayout->addItem(vertical_spacer);
  VBoxLayout->addLayout(HBoxLayout_pushButton_apply);

  HBoxLayout_1->addWidget(label_1);
  HBoxLayout_1->addWidget(PickWidget_1);
  HBoxLayout_2->addWidget(label_2);
  HBoxLayout_2->addWidget(PickWidget_2);
  HBoxLayout_3->addWidget(label_3);
  HBoxLayout_3->addWidget(PickWidget_3);
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
  HBoxLayout_9->addWidget(label_9);
  HBoxLayout_9->addWidget(comboBox_9);

  HBoxLayout_pushButton_apply->addItem(horizontal_spacer_pushButton_apply);
  HBoxLayout_pushButton_apply->addWidget(pushButton_apply);

  QObject::connect(pushButton_apply, SIGNAL(clicked(bool)),this,  SLOT(on_pushButton_apply_clicked(bool)));

  isInitialized = true;
}

LoadsTrajectoryCreatePanel::~LoadsTrajectoryCreatePanel()
{}

void LoadsTrajectoryCreatePanel::on_pushButton_apply_clicked(bool)
{
  QStringList commands;
  QString command = "";

  if ((PickWidget_1->text()!="") && (PickWidget_2->text()!="") && (PickWidget_3->text()!="") && (lineEdit_4->text()!="") && (lineEdit_5->text()!="") && (lineEdit_6->text()!="") && (lineEdit_7->text()!="") && (lineEdit_8->text()!=""))
  {
    command.append("ccx create trajectory curve " + PickWidget_1->text() + " vertex " + PickWidget_2->text()); 

    std::string pickwidget_text = PickWidget_3->text().toUtf8().data();
    std::vector<int> surface_ids = CubitInterface::parse_cubit_list("surface", pickwidget_text);
    command.append(" surface");
    for (size_t i = 0; i < surface_ids.size(); i++)
    {
      command.append(" " + QString::number(surface_ids[i]));
    }
    
    command.append(" direction " + lineEdit_4->text());
    command.append(" magnitude " +lineEdit_5->text());
    command.append(" time_begin " +lineEdit_6->text());
    command.append(" time_end " +lineEdit_7->text());
    command.append(" radius " + lineEdit_8->text());
    
    if (comboBox_9->currentIndex()!=0)
    {
      command.append(" op " + comboBox_9->currentText());
    }
  }
  
  if (command != "")
  {
    commands.push_back(command);
    PickWidget_1->setText("");
    PickWidget_2->setText("");
    PickWidget_3->setText("");
    lineEdit_4->setText("");
    lineEdit_5->setText("");
    lineEdit_6->setText("");    
    lineEdit_7->setText("");
    lineEdit_8->setText("");
    comboBox_9->setCurrentIndex(0);
  }
  
  for (size_t i = 0; i < commands.size(); i++)
  {
    //CubitInterface::cmd(commands[int(i)].toStdString().c_str());
    ccx_iface->cmd(commands[int(i)].toStdString());
  }
}
